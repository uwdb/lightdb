#ifndef VISUALCLOUD_ENCODEWRITER_H
#define VISUALCLOUD_ENCODEWRITER_H

#include <system_error>
#include "VideoEncoder.h"

class EncodeWriter {
public:
    virtual NVENCSTATUS WriteFrame(const EncodeBuffer &buffer);
    virtual NVENCSTATUS WriteFrame(const MotionEstimationBuffer &buffer);
    virtual NVENCSTATUS Flush() = 0;

protected:
    EncodeAPI &api;

    EncodeWriter(EncodeAPI &api): api(api) {}
    EncodeWriter(VideoEncoder &encoder): EncodeWriter(encoder.api()) {}

    virtual NVENCSTATUS WriteFrame(NV_ENC_LOCK_BITSTREAM &bitstream);
    virtual NVENCSTATUS WriteFrame(const void *buffer, const size_t size) = 0;
};



class DescriptorEncodeWriter: public EncodeWriter {
    DescriptorEncodeWriter(EncodeAPI &api, const int descriptor): EncodeWriter(api), descriptor(descriptor) { }

    NVENCSTATUS WriteFrame(const void *buffer, const size_t size) override {
        return write(descriptor, buffer, size) != -1 ? NV_ENC_SUCCESS : NV_ENC_ERR_GENERIC;
    }

    NVENCSTATUS Flush() override { return NV_ENC_SUCCESS; }

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

    NVENCSTATUS WriteFrame(const void *buffer, const size_t size) override {
        return fwrite(buffer, size, 1, file) == size
            ? NV_ENC_SUCCESS
            : NV_ENC_ERR_GENERIC;
    }

    NVENCSTATUS Flush() override {
        return fflush(file) == 0 ? NV_ENC_SUCCESS : NV_ENC_ERR_GENERIC;
    }


private:
    FILE* file;
};

#endif //VISUALCLOUD_ENCODEWRITER_H
