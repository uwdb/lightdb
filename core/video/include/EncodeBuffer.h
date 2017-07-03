#ifndef VISUALCLOUD_ENCODEBUFFER_H
#define VISUALCLOUD_ENCODEBUFFER_H

#include <string>
#include <stdint.h>
#include "EncodeAPI.h"
#include "nvEncodeAPI.h"
#include "nvUtils.h"
#include "GPUContext.h"

typedef struct _EncodeConfig
{
    int              width;
    int              height;
    int              maxWidth;
    int              maxHeight;
    int              fps;
    int              bitrate;
    int              vbvMaxBitrate;
    int              vbvSize;
    int              rcMode;
    int              qp;
    float            i_quant_factor;
    float            b_quant_factor;
    float            i_quant_offset;
    float            b_quant_offset;
    GUID             presetGUID;
    FILE            *fOutput;
    int              codec;
    int              invalidateRefFramesEnableFlag;
    int              intraRefreshEnableFlag;
    int              intraRefreshPeriod;
    int              intraRefreshDuration;
    int              deviceType;
    int              startFrameIdx;
    int              endFrameIdx;
    int              gopLength;
    int              numB;
    int              pictureStruct;
    const unsigned int              deviceID;
    NV_ENC_BUFFER_FORMAT inputFormat;
    char            *qpDeltaMapFile;
    //std::string inputFileName;
    //std::string outputFileName;
    std::string encoderPreset;
    int  enableMEOnly;
    int  enableAsyncMode;
    int  preloadedFrameCount;
    int  enableTemporalAQ;

    _EncodeConfig() :
        EncodeConfig(0, 0, 0, 0, 0, "hq", 0, 0, 0, 0, 0)
        { }

    _EncodeConfig(const struct _EncodeConfig& copy) = default;

    _EncodeConfig(const unsigned int height,
                  const unsigned int width, const size_t tileRows, const size_t tileColumns,
                  const unsigned int codec, std::string preset, const unsigned int fps,
                  const unsigned int gop_length, const size_t bitrate, const unsigned int rcmode,
                  const unsigned int deviceId) :
            height(height),
            width(width),
            maxHeight(0),
            maxWidth(0),
            startFrameIdx(0),
            endFrameIdx(INT_MAX),
            bitrate(bitrate),
            rcMode(rcmode),
            gopLength(gop_length),
            codec(codec),
            fps(fps),
            qp(28),
            i_quant_factor(DEFAULT_I_QFACTOR),
            b_quant_factor(DEFAULT_B_QFACTOR),
            i_quant_offset(DEFAULT_I_QOFFSET),
            b_quant_offset(DEFAULT_B_QOFFSET),
            presetGUID(NV_ENC_PRESET_DEFAULT_GUID),
            pictureStruct(NV_ENC_PIC_STRUCT_FRAME),
            encoderPreset(preset),
            deviceID(deviceId),
            //inputFileName(inputFilename),
            //outputFileName(outputFilenameFormat),
            fOutput(nullptr),
            vbvMaxBitrate(0),
            vbvSize(0),
            invalidateRefFramesEnableFlag(0),
            intraRefreshEnableFlag(0),
            intraRefreshPeriod(0),
            intraRefreshDuration(0),
            deviceType(NV_ENC_DEVICE_TYPE_CUDA),
            numB(0),
            inputFormat(NV_ENC_BUFFER_FORMAT_NV12),
            qpDeltaMapFile(nullptr),
            //inputFilePath(nullptr),
            enableMEOnly(0),
            enableAsyncMode(0),
            preloadedFrameCount(0),
            enableTemporalAQ(0)
    { }


}EncodeConfig;

typedef struct _EncodeInputBuffer
{
    unsigned int      dwWidth;
    unsigned int      dwHeight;
#if defined (NV_WINDOWS)
    IDirect3DSurface9 *pNV12Surface;
#endif
    CUdeviceptr       pNV12devPtr;
    uint32_t          uNV12Stride;
    CUdeviceptr       pNV12TempdevPtr;
    uint32_t          uNV12TempStride;
    void*             nvRegisteredResource;
    NV_ENC_INPUT_PTR  hInputSurface;
    NV_ENC_BUFFER_FORMAT bufferFmt;
} EncodeInputBuffer;

typedef struct _EncodeOutputBuffer
{
    unsigned int          dwBitstreamBufferSize;
    NV_ENC_OUTPUT_PTR     hBitstreamBuffer;
    HANDLE                hOutputEvent;
    bool                  bWaitOnEvent;
    bool                  bEOSFlag;
} EncodeOutputBuffer;

typedef struct _EncodeBuffer
{
    EncodeOutputBuffer      stOutputBfr;
    EncodeInputBuffer       stInputBfr;
    EncodeAPI&              api;
    EncodeConfig&           configuration; // TODO change this to const (possibly others)
    const size_t            size;

    _EncodeBuffer(const _EncodeBuffer& other)
            : _EncodeBuffer(other.api, other.configuration, other.size)
    { }

    _EncodeBuffer(_EncodeBuffer&& other)
            : stOutputBfr(other.stOutputBfr), stInputBfr(other.stInputBfr),
              api(other.api), configuration(other.configuration), size(other.size),
              owner(true) {
        other.owner = false;
    }

    // TODO is this size reasonable?
    _EncodeBuffer(EncodeAPI &api, EncodeConfig& configuration, size_t size=2*1024*1024)
            : stOutputBfr{0}, stInputBfr{0}, api(api), configuration(configuration), size(size), owner(true) {
        NVENCSTATUS status;

        if((status = api.CreateEncoder(&configuration)) != NV_ENC_SUCCESS)
            throw status; //TODO
        else if(cuMemAllocPitch(&stInputBfr.pNV12devPtr,
                           (size_t*)&stInputBfr.uNV12Stride,
                           configuration.width,
                           configuration.height * 3 / 2,
                           16) != CUDA_SUCCESS)
            throw "NV_ENC_ERR_OUT_OF_MEMORY"; //tODO
        else if((status = api.NvEncRegisterResource(
                NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR, (void *)stInputBfr.pNV12devPtr,
                configuration.width, configuration.height,
                stInputBfr.uNV12Stride, &stInputBfr.nvRegisteredResource)) != NV_ENC_SUCCESS)
            throw status; //TODO
        else if((status = api.NvEncMapInputResource(stInputBfr.nvRegisteredResource,
                                                    &stInputBfr.hInputSurface)) != NV_ENC_SUCCESS)
            throw "998"; //TODO
        else if((status = api.NvEncCreateBitstreamBuffer(
                size, &stOutputBfr.hBitstreamBuffer)) != NV_ENC_SUCCESS)
            throw "997"; //TODO

        stInputBfr.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12_PL;
        stInputBfr.dwWidth = configuration.width;
        stInputBfr.dwHeight = configuration.height;
        stOutputBfr.dwBitstreamBufferSize = size;
        stOutputBfr.hOutputEvent = nullptr;
    }

    ~_EncodeBuffer() {
        NVENCSTATUS status;

        if(!owner)
            ;
        else if((status = api.NvEncDestroyBitstreamBuffer(stOutputBfr.hBitstreamBuffer)) != NV_ENC_SUCCESS)
            printf("log\n"); //TODO log
        else if((status = api.NvEncUnmapInputResource(stInputBfr.hInputSurface)) != NV_ENC_SUCCESS)
            printf("log\n"); //TODO log
        else if((api.NvEncUnregisterResource(stInputBfr.nvRegisteredResource)) != NV_ENC_SUCCESS)
            printf("log\n"); //TODO log
        else if(cuMemFree(stInputBfr.pNV12devPtr) != CUDA_SUCCESS)
            printf("log\n"); //TODO log
    }
private:
    bool owner;
} EncodeBuffer;

typedef struct _MotionEstimationBuffer
{
    EncodeOutputBuffer      stOutputBfr;
    EncodeInputBuffer       stInputBfr[2];
    unsigned int            inputFrameIndex;
    unsigned int            referenceFrameIndex;
} MotionEstimationBuffer;

#endif //VISUALCLOUD_ENCODEBUFFER_H
