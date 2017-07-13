#ifndef VISUALCLOUD_ENCODEWRITER_H
#define VISUALCLOUD_ENCODEWRITER_H

#include <system_error>
#include "EncodeBuffer.h"

class EncodeWriter {
public:
    virtual NVENCSTATUS WriteFrame(const EncodeBuffer &buffer);
    virtual NVENCSTATUS WriteFrame(const MotionEstimationBuffer &buffer);

protected:
    EncodeAPI &api;

    EncodeWriter(EncodeAPI &api): api(api) {}
    virtual NVENCSTATUS WriteFrame(NV_ENC_LOCK_BITSTREAM &bitstream);
    virtual NVENCSTATUS WriteFrame(const void *buffer, const size_t size) = 0;
};

class DescriptorEncodeWriter: public EncodeWriter {
    DescriptorEncodeWriter(EncodeAPI &api, const int descriptor): EncodeWriter(api), descriptor(descriptor) { }

     NVENCSTATUS WriteFrame(const void *buffer, const size_t size) override {
        write(descriptor, buffer, size);
    }

private:
    const int descriptor;
};

class FileEncodeWriter: public EncodeWriter {
public:
    FileEncodeWriter(EncodeAPI &api, const std::string &filename)
        : FileEncodeWriter(api, filename.c_str()) { }

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

private:
    FILE* file;
};

#endif //VISUALCLOUD_ENCODEWRITER_H
