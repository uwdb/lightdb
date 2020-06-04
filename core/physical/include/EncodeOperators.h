#ifndef LIGHTDB_ENCODEOPERATORS_H
#define LIGHTDB_ENCODEOPERATORS_H

#include "LightField.h"
#include "PhysicalOperators.h"
#include "Codec.h"
#include "Format.h"
#include "Configuration.h"
#include "VideoEncoder.h"
#include "EncodeWriter.h"
#include "VideoEncoderSession.h"
#include "MaterializedLightField.h"
#include <cstdio>
#include <utility>

extern "C" {
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
};

namespace lightdb::physical {

class GPUEncodeToCPU : public PhysicalOperator, public GPUOperator {
public:
    static constexpr ssize_t kDefaultGopSize = 30;

    explicit GPUEncodeToCPU(const LightFieldReference &logical,
                            PhysicalOperatorReference &parent,
                            Codec codec)
            : PhysicalOperator(logical, {parent}, DeviceType::GPU, runtime::make<Runtime>(*this)),
              GPUOperator(parent),
              codec_(std::move(codec)) {
        if(!codec.nvidiaId().has_value())
            throw GpuRuntimeError("Requested codec does not have an Nvidia encode id");
    }

    const Codec &codec() const { return codec_; }

private:
    class Runtime: public runtime::GPUUnaryRuntime<GPUEncodeToCPU, GPUDecodedFrameData> {
    public:
        explicit Runtime(GPUEncodeToCPU &physical)
            : runtime::GPUUnaryRuntime<GPUEncodeToCPU, GPUDecodedFrameData>(physical),
              encodeConfiguration_{configuration(), this->physical().codec().nvidiaId().value(), gop()},
              encoder_{this->context(), encodeConfiguration_, lock()},
              writer_{encoder_.api()},
              encodeSession_{encoder_, writer_}
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if (iterator() != iterator().eos()) {
                auto decoded = iterator()++;

                for (const auto &frame: decoded.frames())
                    encodeSession_.Encode(*frame, decoded.configuration().offset.top, decoded.configuration().offset.left);

                //TODO think this should move down just above nullopt
                // Did we just reach the end of the decode stream?
                if (iterator() == iterator().eos())
                    // If so, flush the encode queue and end this op too
                    encodeSession_.Flush();

                return {CPUEncodedFrameData(physical().codec(), decoded.configuration(), decoded.geometry(), writer_.dequeue())};
            } else
                return std::nullopt;
        }

    private:
        unsigned int gop() const {
            auto option = logical().is<OptionContainer<>>()
                          ? logical().downcast<OptionContainer<>>().get_option(EncodeOptions::GOPSize)
                          : std::nullopt;

            if(option.has_value() && option.value().type() != typeid(unsigned int))
                throw InvalidArgumentError("Invalid GOP option specified", EncodeOptions::GOPSize);
            else
                return std::any_cast<unsigned int>(option.value_or(
                        std::make_any<unsigned int>(kDefaultGopSize)));
        }

        EncodeConfiguration encodeConfiguration_;
        VideoEncoder encoder_;
        MemoryEncodeWriter writer_;
        VideoEncoderSession encodeSession_;
    };

    const Codec codec_;
};

class CPUEncode: public PhysicalOperator {
    public:
        explicit CPUEncode(const LightFieldReference &logical,
                           const PhysicalOperatorReference &source,
                           Codec codec)
                : PhysicalOperator(logical, {source}, DeviceType::CPU, runtime::make<Runtime>(*this)),
                  codec_(std::move(codec)) {
            CHECK_EQ(source->device(), DeviceType::CPU);
        }

        CPUEncode(const CPUEncode&) = delete;
        CPUEncode(CPUEncode&&) = default;

        const Codec &codec() const { return codec_; }

    private:
        class Runtime: public runtime::UnaryRuntime<CPUEncode, CPUDecodedFrameData> {
        public:
            static constexpr const size_t kDefaultGopSize = 30;
            static constexpr const unsigned int kMaxBFrames = 10;

            explicit Runtime(CPUEncode &physical)
                    : runtime::UnaryRuntime<CPUEncode, CPUDecodedFrameData>(physical),
                      encodeConfiguration_{configuration(), this->physical().codec().nvidiaId().value(), gop()},
                      geometry_{geometry()},
                      codec_(get_codec()),
                      packet_(create_packet()) {
                CHECK_NOTNULL(codec_.get());
                CHECK_NOTNULL(packet_.get());
            }

            Runtime(const Runtime&) = delete;
            Runtime(Runtime&&) = delete;

            std::optional<physical::MaterializedLightFieldReference> read() override {
                if (iterator() != iterator().eos()) {
                    auto decoded = iterator()++;
                    bytestring encoded_data;

                    if(!decoded.frames().empty()) {
                        if(codec_context_ == nullptr) {
                            auto &format = decoded.frames().front()->format();
                            codec_context_ = create_codec_context(format);
                            frame_ = create_frame(codec_context_);
                        }

                        for (const auto &frame: decoded.frames())
                            while(!encode_frame(frame))
                                get_encoded_chunk(encoded_data);

                        if(iterator() == iterator().eos())
                            flush_encoder();

                        while(get_encoded_chunk(encoded_data, iterator() == iterator().eos()));
                    }

                    return {CPUEncodedFrameData(physical().codec(), decoded.configuration(), decoded.geometry(), encoded_data)};
                } else
                    return std::nullopt;
            }

        private:
            unsigned int gop() const {
                auto option = logical().is<OptionContainer<>>()
                              ? logical().downcast<OptionContainer<>>().get_option(EncodeOptions::GOPSize)
                              : std::nullopt;

                if(option.has_value() && option.value().type() != typeid(unsigned int))
                    throw InvalidArgumentError("Invalid GOP option specified", EncodeOptions::GOPSize);
                else
                    return std::any_cast<unsigned int>(option.value_or(
                           std::make_any<unsigned int>(kDefaultGopSize)));
            }

            std::shared_ptr<AVCodec> get_codec() {
                AVCodec *codec;

                if(!physical().codec().ffmpegId().has_value())
                    throw FfmpegRuntimeError("No Ffmpeg codec id associated with requested codec");
                else if((codec = avcodec_find_encoder(static_cast<::AVCodecID>(physical().codec().ffmpegId().value()))) == nullptr)
                    throw FfmpegRuntimeError("Codec not found");

                return std::shared_ptr<AVCodec>(codec, [](auto&) {});
            }

            int configure_codec(AVCodecContext *codec_context, const video::Format &format) {
                codec_context->bit_rate = configuration().bitrate;
                codec_context->width = configuration().width;
                codec_context->height = configuration().height;
                codec_context->framerate = AVRational{static_cast<int>(configuration().framerate.denominator()),
                                                      static_cast<int>(configuration().framerate.numerator())};
                codec_context->time_base = AVRational{static_cast<int>(configuration().framerate.denominator()),
                                                      static_cast<int>(configuration().framerate.numerator())};
                codec_context->gop_size = encodeConfiguration_.gopLength;
                codec_context->max_b_frames = static_cast<int>(encodeConfiguration_.numB != 0
                                                  ? encodeConfiguration_.numB :
                                                  kMaxBFrames);
                codec_context->pix_fmt = static_cast<::AVPixelFormat>(format.ffmpeg_format().value());

                return 0;
            }

            static std::shared_ptr<AVPacket> create_packet() {
                AVPacket *packet;

                if ((packet = av_packet_alloc()) == nullptr)
                    throw FfmpegRuntimeError("Error allocating packet");
                else {
                    av_init_packet(packet);
                    packet->data = nullptr;
                    packet->size = 0;

                    return std::shared_ptr<AVPacket>(packet, [](auto &p) { av_packet_free(&p); });
                }
            }

            static std::shared_ptr<AVFrame> create_frame(const std::shared_ptr<AVCodecContext> &context) {
                AVFrame *frame;

                if ((frame = av_frame_alloc()) == nullptr)
                    throw FfmpegRuntimeError("Error allocating codec context");
                else {
                    frame->format = context->pix_fmt;
                    frame->width  = context->width;
                    frame->height = context->height;
                    frame->pts = 0;

                    return std::shared_ptr<AVFrame>(frame, [](auto &f) { av_frame_free(&f); });
                }
            }

            std::shared_ptr<AVCodecContext> create_codec_context(const video::Format &format) {
                AVCodecContext *codec_context;
                int result;

                if((codec_context = avcodec_alloc_context3(codec_.get())) == nullptr)
                    throw FfmpegRuntimeError("Error allocating codec context");
                else if ((result = configure_codec(codec_context, format)) != 0)
                    throw FfmpegRuntimeError(result);
                else if((result = avcodec_open2(codec_context, codec_.get(), nullptr)) != 0)
                    throw FfmpegRuntimeError(result);
                else
                    return std::shared_ptr<AVCodecContext>(codec_context, avcodec_close);
            }

            bool encode_frame(const LocalFrameReference &frame) {
                int result;

                static std::vector<std::shared_ptr<AVFrame>> frames;

                frame_->pts++;
                if((result = av_image_fill_arrays(frame_->data, frame_->linesize,
                                                       reinterpret_cast<const uint8_t*>(frame->data().data()),
                                                       static_cast<AVPixelFormat >(frame_->format),
                                                       frame_->width, frame_->height, 1)) < 0)
                    throw FfmpegRuntimeError(result);
                else if((result = avcodec_send_frame(codec_context_.get(), frame_.get())) != 0 && result != AVERROR(EAGAIN))
                    throw FfmpegRuntimeError(result);
                else
                    return result == 0;
            }

            void flush_encoder() {
                int result;

                if((result = avcodec_send_frame(codec_context_.get(), nullptr)) != 0 &&
                   result != AVERROR(EAGAIN) && result != AVERROR_EOF)
                    throw FfmpegRuntimeError(result);
                else if(result == AVERROR(EAGAIN))
                    flush_encoder();
            }

            bool get_encoded_chunk(bytestring &data, const bool flushing=false) {
                int result;

                if((result = avcodec_receive_packet(codec_context_.get(), packet_.get())) != 0 &&
                   result != AVERROR(EAGAIN) && result != AVERROR_EOF)
                    throw FfmpegRuntimeError(result);
                else if(result == AVERROR_EOF)
                    return false;
                else if(result == AVERROR(EAGAIN))
                    return flushing;
                else {
                    data.insert(std::end(data), packet_->data, packet_->data + packet_->size);
                    av_packet_unref(packet_.get());
                    return true;
                }
            }

            const EncodeConfiguration encodeConfiguration_;
            const GeometryReference geometry_;
            const std::shared_ptr<AVCodec> codec_;
            const std::shared_ptr<AVPacket> packet_;
            std::shared_ptr<AVCodecContext> codec_context_;
            std::shared_ptr<AVFrame> frame_;
        };

        const Codec codec_;
    };


}; // namespace lightdb::physical

#endif //LIGHTDB_ENCODEOPERATORS_H
