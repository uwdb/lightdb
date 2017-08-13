#ifndef VISUALCLOUD_DECODEREADER_H
#define VISUALCLOUD_DECODEREADER_H

#include <boost/lockfree/spsc_queue.hpp>
#include "dynlink_cuda.h"
#include "PThreadMutex.h"

class DecodeReader {
public:
    virtual std::optional<CUVIDSOURCEDATAPACKET> DecodeFrame() = 0;
    virtual CUVIDEOFORMAT format() const = 0;
};

class FileDecodeReader: public DecodeReader {
public:
    FileDecodeReader(const std::string &filename)
        : FileDecodeReader(filename.c_str()) { }

    FileDecodeReader(const char *filename)
            : filename(filename),
              packets(), // Must be initialized before source
              source(CreateVideoSource(filename)),
              format_(GetVideoSourceFormat(source)) {
        if(format().codec != cudaVideoCodec_H264 &&
                format().codec != cudaVideoCodec_HEVC)
            throw "Reader only supports H264/HEVC input video"; //TODO
        else if(format().chroma_format != cudaVideoChromaFormat_420)
            throw "Reader only supports 4:2:0 chroma"; // TODO
    }

    CUVIDEOFORMAT format() const override { return format_; }

    std::optional<CUVIDSOURCEDATAPACKET> DecodeFrame() override {
        CUVIDSOURCEDATAPACKET packet;

        while (cuvidGetVideoSourceState(source) == cudaVideoState_Started &&
                !packets.read_available())
            sleep(0);

        return packets.pop(packet)
               ? packet
               : std::optional<CUVIDSOURCEDATAPACKET>();
    }

private:
    void Initialize() {
    }

    static int CUDAAPI HandleVideoData(void *userData, CUVIDSOURCEDATAPACKET *packet) {
        FileDecodeReader *reader = static_cast<FileDecodeReader*>(userData);

        while(!reader->packets.push(*packet))
            sleep(0);

        return 1;
    }

    CUvideosource CreateVideoSource(const char *filename) {
        CUresult status;
        CUvideosource source;
        CUVIDSOURCEPARAMS videoSourceParameters = {
                .ulClockRate = 0,
                .uReserved1 = {},
                .pUserData = this,
                .pfnVideoDataHandler = HandleVideoData,
                .pfnAudioDataHandler = nullptr,
                0
        };

        if((status = cuvidCreateVideoSource(&source, filename, &videoSourceParameters)) != CUDA_SUCCESS)
            throw status; //TODO
        else if((status = cuvidSetVideoSourceState(source, cudaVideoState_Started)) != CUDA_SUCCESS)
            throw status; //TODO

        return source;
    }

    CUVIDEOFORMAT GetVideoSourceFormat(CUvideosource source) {
        CUresult status;
        CUVIDEOFORMAT format;

        if((status = cuvidGetSourceVideoFormat(source, &format, 0)) != CUDA_SUCCESS)
            throw status; //TODO
        return format;
    }

    std::string filename;
    boost::lockfree::spsc_queue<CUVIDSOURCEDATAPACKET, boost::lockfree::capacity<4096>> packets;
    CUvideosource source;
    CUVIDEOFORMAT format_;
};

#endif //VISUALCLOUD_DECODEREADER_H
