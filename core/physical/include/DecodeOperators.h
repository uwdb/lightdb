#ifndef LIGHTDB_DECODEOPERATORS_H
#define LIGHTDB_DECODEOPERATORS_H

#include "LightField.h"
#include "Environment.h"
#include "PhysicalOperators.h"
#include "MaterializedLightField.h"
#include "VideoDecoderSession.h"
#include "Runtime.h"

extern "C" {
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/error.h>
};

namespace lightdb::physical {

class GPUDecodeFromCPU : public PhysicalOperator, public GPUOperator {
public:
    explicit GPUDecodeFromCPU(const LightFieldReference &logical,
                              PhysicalOperatorReference source,
                              const execution::GPU &gpu)
            : GPUDecodeFromCPU(logical,
                               source,
                               gpu,
                               std::chrono::milliseconds(1u))
    { }

    template<typename Rep, typename Period>
    explicit GPUDecodeFromCPU(const LightFieldReference &logical,
                              PhysicalOperatorReference &source,
                              const execution::GPU &gpu,
                              std::chrono::duration<Rep, Period> poll_duration)
            : PhysicalOperator(logical, {source}, DeviceType::GPU, runtime::make<Runtime>(*this)),
              GPUOperator(gpu),
              poll_duration_(poll_duration) {
        CHECK_EQ(source->device(), DeviceType::CPU);
    }

    GPUDecodeFromCPU(const GPUDecodeFromCPU&) = delete;
    GPUDecodeFromCPU(GPUDecodeFromCPU&&) = default;

    std::chrono::microseconds poll_duration() const { return poll_duration_; }

private:
    class Runtime: public runtime::GPUUnaryRuntime<GPUDecodeFromCPU, CPUEncodedFrameData> {
    public:
        explicit Runtime(GPUDecodeFromCPU &physical)
            : runtime::GPUUnaryRuntime<GPUDecodeFromCPU, CPUEncodedFrameData>(physical),
              configuration_{configuration(), codec()},
              geometry_{geometry()},
              queue_{lock()},
              decoder_{configuration_, queue_, lock()},
              session_{decoder_, iterator(), iterator().eos()}
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            std::vector<GPUFrameReference> frames;

            LOG_IF(WARNING, configuration_.output_surfaces < 8)
                << "Decode configuration output surfaces is low, limiting throughput";

            if(!decoder_.frame_queue().isComplete())
                do {
                    auto frame = session_.decode(physical().poll_duration());
                    if (frame.has_value())
                        frames.emplace_back(frame.value());
                } while(!decoder_.frame_queue().isEmpty() &&
                        !decoder_.frame_queue().isEndOfDecode() &&
                        frames.size() <= configuration_.output_surfaces / 4);

            if(!frames.empty() || !decoder_.frame_queue().isComplete())
                return std::optional<physical::MaterializedLightFieldReference>{
                          GPUDecodedFrameData(configuration_, geometry_, frames)};
            else
                return std::nullopt;
        }

    private:
        const DecodeConfiguration configuration_;
        const GeometryReference geometry_;
        CUVIDFrameQueue queue_;
        CudaDecoder decoder_;
        VideoDecoderSession<Runtime::downcast_iterator<CPUEncodedFrameData>> session_;
    };

    const std::chrono::microseconds poll_duration_;
};

class CPUDecode : public PhysicalOperator {
public:
    explicit CPUDecode(const LightFieldReference &logical,
                       const PhysicalOperatorReference &source)
            : PhysicalOperator(logical, {source}, DeviceType::CPU, runtime::make<Runtime>(*this)) {
        CHECK_EQ(source->device(), DeviceType::CPU);
    }

    CPUDecode(const CPUDecode&) = delete;
    CPUDecode(CPUDecode&&) = default;

private:
    class Runtime: public runtime::UnaryRuntime<CPUDecode, CPUEncodedFrameData> {
    public:
        explicit Runtime(CPUDecode &physical)
                : runtime::UnaryRuntime<CPUDecode, CPUEncodedFrameData>(physical),
                  configuration_{configuration(), codec()},
                  geometry_{geometry()},
                  buffer_size_(65536),
                  buffer_(reinterpret_cast<unsigned char*>(av_malloc(buffer_size_ + AV_INPUT_BUFFER_PADDING_SIZE)), av_free),
                  context_(avio_alloc_context(buffer_.release(), static_cast<int>(buffer_size_), 0,
                                              this, &read_next_packet, nullptr, nullptr), [](auto &c) {
                                                  avio_context_free(&c); /* avio_close */ }),
                  format_(create_format()),
                  stream_index_(get_stream_index()),
                  codec_(get_codec()),
                  codec_context_(create_codec_context()),
                  frames_(256),
                  packet_worker_(&Runtime::decode_all, this) {
            CHECK_NOTNULL(context_.get());
            CHECK_NOTNULL(format_.get());
            CHECK_NOTNULL(codec_.get());
            CHECK_NOTNULL(codec_context_.get());
        }

        Runtime(const Runtime&) = delete;
        Runtime(Runtime&&) = delete;

        ~Runtime() override {
            decoding_ = false;
            packet_worker_.join();
        }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            CPUDecodedFrameData output(configuration_, geometry_);

            frames_.consume_all([&output](const auto &frame) {
                output.frames().push_back(frame); });

            return decoding_ || !output.frames().empty()
                ? std::optional{output}
                : std::nullopt;
        }

    private:
        std::shared_ptr<AVFormatContext> create_format() {
            std::shared_ptr<AVFormatContext> format(avformat_alloc_context(), [](auto &f) { avformat_close_input(&f); });
            auto pointer = format.get();
            int result;

            format->pb = context_.get();

            if((result = avformat_open_input(&pointer, ".mp4", nullptr, nullptr)) != 0)
                throw get_ffmpeg_error(result);
            else
                return format;
        }

        unsigned int get_stream_index() {
            int result;
            if ((result = avformat_find_stream_info(format_.get(), nullptr)) != 0)
                throw get_ffmpeg_error(result);
            else if((result = av_find_best_stream(format_.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0)) < 0)
                throw get_ffmpeg_error(result);
            else
                return static_cast<unsigned int>(result);
        }

        std::shared_ptr<AVCodec> get_codec() {
            AVCodec *codec;

            CHECK_EQ(av_find_best_stream(format_.get(), AVMEDIA_TYPE_VIDEO, stream_index_, -1, &codec, 0),
                     stream_index_);

            return std::shared_ptr<AVCodec>(codec, [](auto&) {});
        }

        std::shared_ptr<AVCodecContext> create_codec_context() {
            auto parameters = format_->streams[stream_index_]->codecpar;
            AVCodecContext *codec_context;
            int result;

            if((codec_context = avcodec_alloc_context3(codec_.get())) == nullptr)
                throw FfmpegRuntimeError("Error allocating codec context");
            else if ((result = avcodec_parameters_to_context(codec_context, parameters)) != 0)
                throw get_ffmpeg_error(result);
            else if((result = avcodec_open2(codec_context, codec_.get(), nullptr)) != 0)
                throw get_ffmpeg_error(result);
            else
                return std::shared_ptr<AVCodecContext>(codec_context, avcodec_close);
        }

        int send_frames(const std::shared_ptr<AVFrame>& frame) {
            int result;

            switch(result = avcodec_receive_frame(codec_context_.get(), frame.get())) {
                case 0: {
                    CHECK_EQ(frame->width, configuration_.width);
                    CHECK_EQ(frame->height, configuration_.height);
                    CHECK_EQ(frame->format, AV_PIX_FMT_YUV420P);

                    auto format = static_cast<AVPixelFormat>(frame->format);
                    auto size = av_image_get_buffer_size(format, frame->width, frame->height, 1);
                    auto buffer = std::make_shared<bytestring>(size);
                    if((result = av_image_copy_to_buffer(
                            reinterpret_cast<unsigned char *>(buffer->data()), buffer->size(), frame->data,
                            frame->linesize, format, frame->width, frame->height, 1)) != size)
                            throw get_ffmpeg_error(result);

                    frames_.push(std::make_shared<LocalFrame>(frame->height, frame->width, buffer));
                    break;
                }
                case AVERROR(EAGAIN):
                    break;
                case AVERROR_EOF:
                    decoding_ = false;
                    break;
                default:
                    throw get_ffmpeg_error(result);
            }

            return result;
        }

        void decode_all() {
            const std::shared_ptr<AVPacket> packet(av_packet_alloc(), [](auto &p) { av_packet_free(&p); });
            const std::shared_ptr<AVFrame> frame(av_frame_alloc(), [](auto &f) { av_frame_free(&f); });
            int result;

            while(decoding_) {
                // Read and send packet
                switch(result = av_read_frame(format_.get(), packet.get())) {
                    case 0:
                        if((result = avcodec_send_packet(codec_context_.get(), packet.get())) != 0)
                            throw get_ffmpeg_error(result);
                        break;
                    case AVERROR_EOF:
                        if((result = avcodec_send_packet(codec_context_.get(), nullptr)) != 0 &&
                           result != AVERROR_EOF)
                            throw get_ffmpeg_error(result);
                        break;
                    default:
                        throw get_ffmpeg_error(result);
                }

                // Read and send frames
                while(send_frames(frame) == 0)
                    ;
            }
        }

        static int read_next_packet(void* opaque, unsigned char* buffer, int size) {
            auto &runtime = *static_cast<Runtime*>(opaque);

            if(runtime.iterator() == runtime.iterator().eos()) {
                return AVERROR_EOF;
            } else {
                auto data = runtime.iterator()++;

                CHECK_GE(size, data.value().size());
                CHECK_LE(data.value().size(), std::numeric_limits<int>::max());

                std::copy(
                        std::begin(data.value()),
                        std::end(data.value()),
                        buffer);

                return data.value().size() != 0
                       ? static_cast<int>(data.value().size())
                       : read_next_packet(opaque, buffer, size);
            }
        }

        std::exception get_ffmpeg_error(int error) {
            char message[256];
            av_strerror(error, message, sizeof(message));
            return FfmpegRuntimeError(message);
        }

        const DecodeConfiguration configuration_;
        const GeometryReference geometry_;
        const size_t buffer_size_;
        std::unique_ptr<unsigned char, void(*)(void*)> buffer_;
        const std::shared_ptr<AVIOContext> context_;
        const std::shared_ptr<AVFormatContext> format_;
        const unsigned int stream_index_;
        const std::shared_ptr<AVCodec> codec_;
        const std::shared_ptr<AVCodecContext> codec_context_;

        lightdb::spsc_queue<std::shared_ptr<LocalFrame>> frames_;
        bool decoding_ = true;
        std::thread packet_worker_;
    };
};

template<typename T>
class CPUFixedLengthRecordDecode : public PhysicalOperator {
public:
    CPUFixedLengthRecordDecode(const LightFieldReference &logical,
                               const PhysicalOperatorReference &source)
            : PhysicalOperator(logical, {source}, DeviceType::CPU, runtime::make<Runtime>(*this)) {
        CHECK_EQ(source->device(), DeviceType::CPU);
    }

    CPUFixedLengthRecordDecode(const CPUFixedLengthRecordDecode &) = delete;
    CPUFixedLengthRecordDecode(CPUFixedLengthRecordDecode&&) noexcept = default;

private:
    class Runtime: public runtime::UnaryRuntime<CPUFixedLengthRecordDecode, CPUEncodedFrameData> {
    public:
        explicit Runtime(CPUFixedLengthRecordDecode &physical)
            : runtime::UnaryRuntime<CPUFixedLengthRecordDecode, CPUEncodedFrameData>(physical)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(this->iterator() != this->iterator().eos()) {
                auto data = this->iterator()++;
                CPUDecodedFrameData output{data.configuration(), data.geometry()};

                for(auto *current = data.value().data(),
                            *end = current + data.value().size();
                    current < end;
                    current += sizeof(T))
                    output.frames().emplace_back(LocalFrameReference::make<LocalFrame>(
                            0u, 0u, lightdb::bytestring(current, current + sizeof(T))));

                return output;
            } else
                return {};
        }
    };
};

} // namespace lightdb::physical

#endif //LIGHTDB_DECODEOPERATORS_H
