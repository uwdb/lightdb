#ifndef LIGHTDB_ENCODEWRITER_H
#define LIGHTDB_ENCODEWRITER_H

#include "VideoEncoder.h"
#include "EncodeBuffer.h"
#include <system_error>

class EncodeWriter {
public:
    virtual NVENCSTATUS WriteFrame(const EncodeBuffer &buffer) {
        NVENCSTATUS result;
        NV_ENC_LOCK_BITSTREAM bitstream{
                .version = NV_ENC_LOCK_BITSTREAM_VER,
                .doNotWait = 0,
                .ltrFrame = 0,
                .reservedBitFields = 0,
                .outputBitstream = buffer.output_buffer.bitstreamBuffer
        };

        if (buffer.output_buffer.bitstreamBuffer == nullptr && !buffer.output_buffer.EOSFlag) {
            result = NV_ENC_ERR_INVALID_PARAM;
        } else if (buffer.output_buffer.waitOnEvent && buffer.output_buffer.outputEvent != nullptr) {
            result = NV_ENC_ERR_INVALID_PARAM;
        } else if (buffer.output_buffer.EOSFlag) {
            result = NV_ENC_SUCCESS;
        } else {
            result = WriteFrame(bitstream) == 0 ? NV_ENC_SUCCESS : NV_ENC_ERR_GENERIC;
        }

        return result;
    }

    virtual NVENCSTATUS Flush() = 0;

protected:
    EncodeAPI &api;

    explicit EncodeWriter(EncodeAPI &api): api(api) {}

    explicit EncodeWriter(VideoEncoder &encoder): EncodeWriter(encoder.api()) {}

    EncodeWriter(const EncodeWriter&) = default;
    EncodeWriter(EncodeWriter&&) noexcept = default;

    virtual NVENCSTATUS WriteFrame(const void *buffer, size_t size) = 0;
    virtual NVENCSTATUS WriteFrame(NV_ENC_LOCK_BITSTREAM &bitstream) {
        NVENCSTATUS status;

        if((status = api.NvEncLockBitstream(&bitstream)) != NV_ENC_SUCCESS) {
            return status;
        }

        status = WriteFrame(bitstream.bitstreamBufferPtr, bitstream.bitstreamSizeInBytes) == 0
                 ? NV_ENC_SUCCESS : NV_ENC_ERR_GENERIC;

        auto unlockStatus = api.NvEncUnlockBitstream(bitstream.outputBitstream);

        return status != NV_ENC_SUCCESS ? status : unlockStatus;
    }
};


class MemoryEncodeWriter: public EncodeWriter {
public:
    explicit MemoryEncodeWriter(EncodeAPI &api, size_t initial_buffer_size=16*1024*1024)
        : EncodeWriter(api), buffer_() {
        buffer_.reserve(initial_buffer_size);
    }

    MemoryEncodeWriter(const MemoryEncodeWriter&) = delete;
    MemoryEncodeWriter(MemoryEncodeWriter&& other) noexcept
            : EncodeWriter(std::move(other)), buffer_(std::move(other.buffer_)), lock_{}
    { }

    NVENCSTATUS Flush() override { return NV_ENC_SUCCESS; }
    const std::vector<char>& buffer() const { return buffer_; }
    std::vector<char> dequeue() {
        std::vector<char> value;
        std::lock_guard lock{lock_};
        buffer_.swap(value);
        return value;
    }

protected:
    NVENCSTATUS WriteFrame(const void *buffer, const size_t size) override {
        std::lock_guard lock{lock_};
        buffer_.insert(buffer_.end(), static_cast<const char*>(buffer), static_cast<const char*>(buffer) + size);
        return NV_ENC_SUCCESS;
    }

private:
    std::vector<char> buffer_;
    std::mutex lock_;
};


class SegmentedMemoryEncodeWriter: public EncodeWriter {
public:
    SegmentedMemoryEncodeWriter(EncodeAPI &api, EncodeConfiguration &configuration,
                                size_t initial_buffer_size=16*1024*1024)
            : SegmentedMemoryEncodeWriter(api, configuration.gopLength, initial_buffer_size)
    { }

    SegmentedMemoryEncodeWriter(EncodeAPI &api, size_t gop_length, size_t initial_buffer_size=16*1024*1024)
            : EncodeWriter(api), gop_length_(gop_length), buffer_(), offsets_(1, 0), writes_(0)
    {
        buffer_.reserve(initial_buffer_size);
        offsets_.reserve(360);
    }

    NVENCSTATUS Flush() override { return NV_ENC_SUCCESS; }
    const std::vector<char> buffer() const { return buffer_; }

    const std::vector<char> segment(size_t index) {
        return {};
    }

protected:
    NVENCSTATUS WriteFrame(const void *buffer, const size_t size) override {
        buffer_.insert(buffer_.end(), static_cast<const char*>(buffer), static_cast<const char*>(buffer) + size);
        if(++writes_ % gop_length_ == 0)
            offsets_.emplace_back(buffer_.size());
        return NV_ENC_SUCCESS;
    }

private:
    size_t gop_length_;
    std::vector<char> buffer_;
    std::vector<off_t> offsets_;
    off_t writes_;
};

class DescriptorEncodeWriter: public EncodeWriter {
public:
    DescriptorEncodeWriter(EncodeAPI &api, const int descriptor): EncodeWriter(api), descriptor(descriptor) { }

    NVENCSTATUS Flush() override { return NV_ENC_SUCCESS; }

protected:
    NVENCSTATUS WriteFrame(const void *buffer, const size_t size) override {
        return write(descriptor, buffer, size) != -1 ? NV_ENC_SUCCESS : NV_ENC_ERR_GENERIC;
    }


private:
    const int descriptor;
};



class FileEncodeWriter: public EncodeWriter {
public:
    FileEncodeWriter(VideoEncoder &encoder, const std::string &filename)
        : FileEncodeWriter(encoder.api(), filename) { }

    FileEncodeWriter(EncodeAPI &api, const std::string &filename)
        : FileEncodeWriter(api, filename.c_str()) { }

    FileEncodeWriter(VideoEncoder &encoder, const char *filename)
        : FileEncodeWriter(encoder.api(), filename) { }

    FileEncodeWriter(EncodeAPI &api, const char *filename)
            : FileEncodeWriter(api, fopen(filename, "wb")) {
        if(file == nullptr)
            throw std::system_error(EFAULT, std::system_category());
    }

    FileEncodeWriter(EncodeAPI &api, FILE *file): EncodeWriter(api), file(file) { }

    ~FileEncodeWriter() {
        if(file != nullptr)
            fclose(file);
    }

    NVENCSTATUS Flush() override {
        return fflush(file) == 0 ? NV_ENC_SUCCESS : NV_ENC_ERR_GENERIC;
    }

protected:
    NVENCSTATUS WriteFrame(const void *buffer, const size_t size) override {
        return fwrite(buffer, size, 1, file) == size
               ? NV_ENC_SUCCESS
               : NV_ENC_ERR_GENERIC;
    }

private:
    FILE* file;
};

#endif //LIGHTDB_ENCODEWRITER_H
