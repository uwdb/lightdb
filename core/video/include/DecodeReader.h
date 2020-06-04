#ifndef LIGHTDB_DECODEREADER_H
#define LIGHTDB_DECODEREADER_H

#include "Format.h"
#include "Ffmpeg.h"
#include "spsc_queue.h"
#include <dynlink_nvcuvid.h>
#include <thread>
#include <fstream>
#include <experimental/filesystem>

namespace lightdb::video {

    struct DecodeReaderPacket: public CUVIDSOURCEDATAPACKET {
public:
    DecodeReaderPacket() : CUVIDSOURCEDATAPACKET{} { }

    DecodeReaderPacket(const DecodeReaderPacket &packet) = default;

    explicit DecodeReaderPacket(const CUVIDSOURCEDATAPACKET &packet)
        : CUVIDSOURCEDATAPACKET{packet.flags, packet.payload_size, nullptr, packet.timestamp},
          buffer_(std::make_shared<std::vector<unsigned char>>()) {
        buffer_->reserve(payload_size);
        buffer_->insert(buffer_->begin(), packet.payload, packet.payload + payload_size);
        payload = buffer_->data();
    }

    explicit DecodeReaderPacket(const std::vector<char> &data, const unsigned long flags=0,
                                const CUvideotimestamp timestamp=0)
            : DecodeReaderPacket(CUVIDSOURCEDATAPACKET{flags, data.size(),
                                                       reinterpret_cast<const unsigned char*>(data.data()), timestamp})
    { }

    DecodeReaderPacket& operator=(const DecodeReaderPacket &packet) = default;
    bool operator==(const DecodeReaderPacket &packet) const noexcept {
        return this->payload_size == packet.payload_size &&
               this->flags == packet.flags &&
               this->timestamp == packet.timestamp &&
               this->buffer_ == packet.buffer_;
    }

private:
    std::shared_ptr<std::vector<unsigned char>> buffer_;
};

class DecodeReader {
public:
    class iterator {
        friend class DecodeReader;

    public:
        bool operator==(const iterator& other) const { return (eos_ && other.eos_) ||
                                                              (reader_ == other.reader_ &&
                                                               *current_ == *other.current_); }
        bool operator!=(const iterator& other) const { return !(*this == other); }
        void operator++()
        {
            if (!(current_ = reader_->read()).has_value())
                eos_ = true;
        }
        const DecodeReaderPacket operator++(int)
        {
            auto value = **this;
            ++*this;
            return value;
        }
        DecodeReaderPacket operator*() { return current_.value(); }

    protected:
        explicit iterator(DecodeReader &reader)
                : reader_(&reader), current_({reader.read()}), eos_(false)
        { }
        constexpr explicit iterator()
                : reader_(nullptr), current_(), eos_(true)
        { }

    private:
        DecodeReader *reader_;
        std::optional<DecodeReaderPacket> current_;
        bool eos_;
    };

    virtual iterator begin() { return iterator(*this); }
    virtual iterator end() { return iterator(); }

    virtual std::optional<DecodeReaderPacket> read() = 0;
    virtual const lightdb::Codec &codec() const = 0;
    virtual bool is_complete() const = 0;
};

class FileDecodeReader: public DecodeReader {
public:
    explicit FileDecodeReader(const std::string &filename)
        : FileDecodeReader(filename.c_str()) { }

    explicit FileDecodeReader(const char *filename)
            : filename_(filename),
              packets_(std::make_unique<lightdb::spsc_queue<DecodeReaderPacket>>(4096)), // Must be initialized before source
              source_(CreateVideoSource(filename)),
              format_(GetVideoSourceFormat(source_)),
              codec_(lightdb::Codec::getByCudaId(format_.codec).value_or(lightdb::Codec::raw())),
              decoded_bytes_(0) {
        if(format_.codec != cudaVideoCodec_H264 && format_.codec != cudaVideoCodec_HEVC)
            throw GpuRuntimeError("FileDecodeReader only supports H264/HEVC input video");
        else if(format_.chroma_format != cudaVideoChromaFormat_420)
            throw GpuRuntimeError("FileDecodeReader only supports 4:2:0 chroma");
    }

    FileDecodeReader(const FileDecodeReader&) = delete;
    FileDecodeReader(FileDecodeReader&& other) noexcept
        : filename_(std::move(other.filename_)),
          packets_(std::move(other.packets_)),
          source_(other.source_),
          format_(other.format_),
          codec_(other.codec_),
          decoded_bytes_(other.decoded_bytes_) {
          other.source_ = nullptr;
    }

    ~FileDecodeReader() {
        CUresult status;

        if(source_ == nullptr)
            ;
        else if(!CompleteVideo())
            LOG(ERROR) << "Swallowed CompleteVideo failure";
        else if((status = cuvidSetVideoSourceState(source_, cudaVideoState_Stopped)) != CUDA_SUCCESS)
            LOG(ERROR) << "Swallowed cuvidSetVideoSourceState failure (" << status << ')';
        else if((status = cuvidDestroyVideoSource(source_)) != CUDA_SUCCESS)
            LOG(ERROR) << "Swallowed cuvidDestroyVideoSource failure (" << status << ')';
        else
            source_ = nullptr;
    }

    inline const lightdb::Codec &codec() const override { return codec_; }
    //inline CUVIDEOFORMAT format() const override { return format_; }
    inline const std::string &filename() const { return filename_; }

    std::optional<DecodeReaderPacket> read() override {
        DecodeReaderPacket packet;

        while (cuvidGetVideoSourceState(source_) == cudaVideoState_Started &&
                !packets_->read_available())
            std::this_thread::yield();

        if(packets_->pop(packet)) {
            decoded_bytes_ += packet.payload_size;
            return {packet};
        } else {
            LOG(INFO) << "Decoded " << decoded_bytes_ << " bytes from " << filename();
            return {};
        }
    }

    inline bool is_complete() const override {
        return !packets_->read_available() && cuvidGetVideoSourceState(source_) != cudaVideoState_Started;
    }

private:
    static int CUDAAPI HandleVideoData(void *userData, CUVIDSOURCEDATAPACKET *packet) {
        auto *packets = static_cast<lightdb::spsc_queue<DecodeReaderPacket>*>(userData);

        while(!packets->push(DecodeReaderPacket(*packet)))
            std::this_thread::yield();

        return 1;
    }

    CUvideosource CreateVideoSource(const char *filename) {
        CUresult status;
        CUvideosource source;
        CUVIDSOURCEPARAMS videoSourceParameters = {
                .ulClockRate = 0,
                .uReserved1 = {},
                .pUserData = packets_.get(),
                .pfnVideoDataHandler = HandleVideoData,
                .pfnAudioDataHandler = nullptr,
                {nullptr}
        };

        if(!std::experimental::filesystem::exists(filename))
            throw InvalidArgumentError("File does not exist", "filename");
        else if(GPUContext::device_count() == 0)
            throw GpuCudaRuntimeError("No CUDA device was found", CUDA_ERROR_NOT_INITIALIZED);
        else if((status = cuvidCreateVideoSource(&source, filename, &videoSourceParameters)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Call to cuvidCreateVideoSource failed", status);
        else if((status = cuvidSetVideoSourceState(source, cudaVideoState_Started)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Call to cuvidSetVideoSourceState failed", status);

        return source;
    }

    CUVIDEOFORMAT GetVideoSourceFormat(CUvideosource source) {
        CUresult status;
        CUVIDEOFORMAT format;

        if((status = cuvidGetSourceVideoFormat(source, &format, 0)) != CUDA_SUCCESS)
            throw GpuCudaRuntimeError("Call to cuvidGetSourceVideoFormat failed", status);
        return format;
    }

    bool CompleteVideo() {
        packets_->reset();
        return true;
    }

    std::string filename_;
    std::unique_ptr<lightdb::spsc_queue<DecodeReaderPacket>> packets_;
    CUvideosource source_;
    CUVIDEOFORMAT format_;
    const lightdb::Codec codec_;
    size_t decoded_bytes_;
};

class FfmpegFileDecodeReader: public DecodeReader {
public:
    explicit FfmpegFileDecodeReader(const std::string &filename, const size_t buffer_size=1024*1024)
            : FfmpegFileDecodeReader(filename.c_str()) { }

    explicit FfmpegFileDecodeReader(const char *filename, const size_t buffer_size=1024*1024)
            : filename_(filename),
              buffer_size_(buffer_size),
              codec_(get_codec(filename)),
              stream_(std::make_unique<std::ifstream>(filename)),
              decoded_bytes_(0) {
        buffer_.resize(buffer_size_);
    }

    FfmpegFileDecodeReader(const FfmpegFileDecodeReader&) = delete;
    FfmpegFileDecodeReader(FfmpegFileDecodeReader&& other) noexcept
            : filename_(other.filename_),
              codec_(other.codec_),
              buffer_size_(other.buffer_size_),
              stream_(other.stream_.release()),
              decoded_bytes_(other.decoded_bytes_) {
        buffer_.resize(buffer_size_);
    }

    ~FfmpegFileDecodeReader() {
        stream_ = nullptr;
    }

    inline const std::string &filename() const { return filename_; }
    inline const Codec &codec() const override { return codec_; }

    std::optional<DecodeReaderPacket> read() override {
        if(!stream_->eof()) {
            stream_->read(buffer_.data(), buffer_.size());
            buffer_.resize(static_cast<unsigned long>(stream_->gcount()));
            decoded_bytes_ += stream_->gcount();

            return DecodeReaderPacket{buffer_};
        } else {
            LOG(INFO) << "Decoded " << decoded_bytes_ << " bytes from " << filename();
            return {};
        }
    }

    inline bool is_complete() const override {
        return stream_->eof();
    }

private:
    static Codec get_codec(const char *filename) {
        auto configuration = ffmpeg::GetStreamConfigurations(filename, true);
        CHECK_EQ(configuration.size(), 1);
        return configuration.front().decode.codec;
    }

    const std::string filename_;
    const size_t buffer_size_;
    std::vector<char> buffer_;
    const Codec codec_;
    std::unique_ptr<std::ifstream> stream_;
    size_t decoded_bytes_;
};

} // namespace lightdb::video

#endif //LIGHTDB_DECODEREADER_H
