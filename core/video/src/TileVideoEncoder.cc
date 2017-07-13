#include <string>
#include "TileVideoEncoder.h"
#include "dynlink_cuda.h" // <cuda.h>
#include <cstdio>
#include "nvEncodeAPI.h"
#include <bits/errno.h>

#define BITSTREAM_BUFFER_SIZE 2*1024*1024

template<typename TCode, typename TReturn>
TReturn error(const char* component, const TCode code, const TReturn result)
{
    fprintf(stderr, "CUDA error %d in %s", code, component);
    return result;
}

template<typename TCode>
TCode error(const char* component, const TCode code)
{
    return error(component, code, code);
}

/*
NVENCSTATUS TileVideoEncoder::Initialize(void* device, const NV_ENC_DEVICE_TYPE deviceType)
{
    NVENCSTATUS status;

    for(TileEncodeContext& context: tileEncodeContext)
        if((status = context.hardwareEncoder.Initialize(device, deviceType)) != NV_ENC_SUCCESS)
            return status;

    return NV_ENC_SUCCESS;
}*/

NVENCSTATUS TileVideoEncoder::CreateEncoders(const std::string &filenameTemplate, EncodeConfig& rootConfiguration)
{
    NVENCSTATUS status;
    //std::string f = filenameTemplate; //rootConfiguration.outputFileName;

    assert(tileDimensions.count);

    for(int i = 0; i < tileDimensions.count; i++)
        {
        auto tileConfiguration = rootConfiguration;
        auto tileFilename(filenameTemplate);
        tileFilename.replace(filenameTemplate.find('%'), 2, std::to_string(i));

        tileConfiguration.width = rootConfiguration.width / tileDimensions.columns;
        tileConfiguration.height = rootConfiguration.height / tileDimensions.rows;

        if((tileConfiguration.fOutput = fopen(tileFilename.c_str(), "wb")) == NULL)
            return error(tileFilename.c_str(), errno, NV_ENC_ERR_GENERIC);
        else if((status = tileEncodeContext[i].hardwareEncoder.CreateEncoder(&tileConfiguration)))
            return status;
        }

    presetGUID = tileEncodeContext[0].hardwareEncoder.GetPresetGUID(
            rootConfiguration.encoderPreset.c_str(), rootConfiguration.codec);

    return NV_ENC_SUCCESS;
}

NVENCSTATUS TileVideoEncoder::AllocateIOBuffers(const EncodeConfig* configuration)
{
    encodeBufferSize = configuration->numB + 4;

    for(TileEncodeContext& context: tileEncodeContext)
    {
        context.encodeBufferQueue.Initialize(context.encodeBuffer.data(), encodeBufferSize);
        AllocateIOBuffer(context, *configuration);
    }

    return NV_ENC_SUCCESS;
}

NVENCSTATUS TileVideoEncoder::AllocateIOBuffer(TileEncodeContext& context, const EncodeConfig& configuration)
{
    NVENCSTATUS status;
    CUresult result;
    auto tileWidth  = configuration.width / tileDimensions.columns;
    auto tileHeight = configuration.height / tileDimensions.rows;

    for (auto i = 0; i < encodeBufferSize; i++) {
        auto& buffer = context.encodeBuffer[i];

        if((result = cuvidCtxLock(lock, 0)) != CUDA_SUCCESS)
            return error("cuvidCtxLock", result, NV_ENC_ERR_GENERIC);
        else if((result = (cuMemAllocPitch(
                &buffer.stInputBfr.pNV12devPtr,
                (size_t*)&buffer.stInputBfr.uNV12Stride,
                tileWidth,
                tileHeight * (3/2) * 2, 16))) != CUDA_SUCCESS)
            return error("cuMemAllocPitch", result, NV_ENC_ERR_GENERIC);
        else if((result = cuvidCtxUnlock(lock, 0)) != CUDA_SUCCESS)
            return error("cuvidCtxUnlock", result, NV_ENC_ERR_GENERIC);
        else if((status = context.hardwareEncoder.NvEncRegisterResource(
                NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR,
                (void*)buffer.stInputBfr.pNV12devPtr,
                tileWidth, tileHeight,
                buffer.stInputBfr.uNV12Stride,
                &buffer.stInputBfr.nvRegisteredResource)) != NV_ENC_SUCCESS)
            return error("NvEncRegisterResource", status);
        else if((status = context.hardwareEncoder.NvEncCreateBitstreamBuffer(
            BITSTREAM_BUFFER_SIZE,
            &buffer.stOutputBfr.hBitstreamBuffer)) != NV_ENC_SUCCESS)
            return error("NvEncCreateBitstreamBuffer", status);
        else
        {
            buffer.stInputBfr.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12_PL;
            buffer.stInputBfr.dwWidth = tileWidth;
            buffer.stInputBfr.dwHeight = tileHeight;
            buffer.stOutputBfr.dwBitstreamBufferSize = BITSTREAM_BUFFER_SIZE;
            buffer.stOutputBfr.hOutputEvent = NULL;
        }
    }
}

NVENCSTATUS TileVideoEncoder::ReleaseIOBuffers()
{
    CUresult result;

    for(TileEncodeContext& context: tileEncodeContext)
        for (auto i = 0; i < encodeBufferSize; i++)
        {
            auto& buffer = context.encodeBuffer[i];

            if((result = cuvidCtxLock(lock, 0)) != CUDA_SUCCESS)
                return error("cuvidCtxLock", result, NV_ENC_ERR_GENERIC);
            else if((result = cuMemFree(buffer.stInputBfr.pNV12devPtr)) != CUDA_SUCCESS)
                return error("cuMemFree", result, NV_ENC_ERR_GENERIC);
            else if((result = cuvidCtxUnlock(lock, 0)) != CUDA_SUCCESS)
                return error("cuvidCtxUnlock", result, NV_ENC_ERR_GENERIC);
            else
            {
                context.hardwareEncoder.NvEncDestroyBitstreamBuffer(buffer.stOutputBfr.hBitstreamBuffer);
                buffer.stOutputBfr.hBitstreamBuffer = NULL;
            }
        }

    return NV_ENC_SUCCESS;
}

NVENCSTATUS TileVideoEncoder::FlushEncoder()
{
    NVENCSTATUS status;

    for(TileEncodeContext& context: tileEncodeContext)
    {
        if((status = context.hardwareEncoder.NvEncFlushEncoderQueue(NULL)) != NV_ENC_SUCCESS)
            return status;

        EncodeBuffer *encodeBuffer = context.encodeBufferQueue.GetPending();
        while (encodeBuffer)
        {
            context.hardwareEncoder.ProcessOutput(context.configuration.fOutput, encodeBuffer);
            encodeBuffer = context.encodeBufferQueue.GetPending();

            if (encodeBuffer && encodeBuffer->stInputBfr.hInputSurface)
            {
                status = context.hardwareEncoder.NvEncUnmapInputResource(encodeBuffer->stInputBfr.hInputSurface);
                encodeBuffer->stInputBfr.hInputSurface = NULL;
            }
        }
    }

    return status;
}

/*
NVENCSTATUS TileVideoEncoder::Deinitialize()
{
    NVENCSTATUS status;

    //ReleaseIOBuffers();

    //for(TileEncodeContext& context: tileEncodeContext)
    //    if((status = context.hardwareEncoder.NvEncDestroyEncoder()) != NV_ENC_SUCCESS)
    //        return status;

    return NV_ENC_SUCCESS;
}*/

EncodeBuffer* GetEncodeBuffer(TileEncodeContext& context)
{
    EncodeBuffer *encodeBuffer = context.encodeBufferQueue.GetAvailable();
    if (!encodeBuffer)
    {
        encodeBuffer = context.encodeBufferQueue.GetPending();
        context.hardwareEncoder.ProcessOutput(context.configuration.fOutput, encodeBuffer);

        // UnMap the input buffer after frame done
        if (encodeBuffer->stInputBfr.hInputSurface)
        {
            context.hardwareEncoder.NvEncUnmapInputResource(encodeBuffer->stInputBfr.hInputSurface);
            encodeBuffer->stInputBfr.hInputSurface = NULL;
        }
        encodeBuffer = context.encodeBufferQueue.GetAvailable();
    }

    return encodeBuffer;
}

NVENCSTATUS TileVideoEncoder::EncodeFrame(EncoderSessionInputFrame *inputFrame,
                                      const NV_ENC_PIC_STRUCT inputFrameType, const bool flush)
{
    NVENCSTATUS status;
    CUresult result;

    if (flush)
        return FlushEncoder();

    assert(inputFrame);
    auto screenWidth = inputFrame->width;
    auto screenHeight = inputFrame->height;
    auto tileWidth = screenWidth / tileDimensions.columns;
    auto tileHeight = screenHeight / tileDimensions.rows;

    for(auto i = 0; i < tileDimensions.count; i++)
    {
        auto& context = tileEncodeContext[i];
        auto* encodeBuffer = GetEncodeBuffer(context);

        auto row = i / tileDimensions.columns;
        auto column = i % tileDimensions.columns;

        auto offsetX = column * tileWidth;
        auto offsetY = row * tileHeight;

        CUDA_MEMCPY2D lumaPlaneParameters = {
            srcXInBytes:   offsetX,
            srcY:          offsetY,
            srcMemoryType: CU_MEMORYTYPE_DEVICE,
            srcHost:       NULL,
            srcDevice:     inputFrame->handle,
            srcArray:      NULL,
            srcPitch:      inputFrame->pitch,

            dstXInBytes:   0,
            dstY:          0,
            dstMemoryType: CU_MEMORYTYPE_DEVICE,
            dstHost:       NULL,
            dstDevice:     (CUdeviceptr)encodeBuffer->stInputBfr.pNV12devPtr,
            dstArray:      NULL,
            dstPitch:      encodeBuffer->stInputBfr.uNV12Stride,

            WidthInBytes:  tileWidth,
            Height:        tileHeight,
            };

        CUDA_MEMCPY2D chromaPlaneParameters = {
            srcXInBytes:   offsetX,
            srcY:          screenHeight + offsetY/2,
            srcMemoryType: CU_MEMORYTYPE_DEVICE,
            srcHost:       NULL,
            srcDevice:     inputFrame->handle,
            srcArray:      NULL,
            srcPitch:      inputFrame->pitch,

            dstXInBytes:   0,
            dstY:          tileHeight,
            dstMemoryType: CU_MEMORYTYPE_DEVICE,
            dstHost:       NULL,
            dstDevice:     (CUdeviceptr)encodeBuffer->stInputBfr.pNV12devPtr,
            dstArray:      NULL,
            dstPitch:      encodeBuffer->stInputBfr.uNV12Stride,

            WidthInBytes:  tileWidth,
            Height:        tileHeight/2
            };

        if((result = cuvidCtxLock(lock, 0)) != CUDA_SUCCESS)
            return error("cuvidCtxLock", result, NV_ENC_ERR_GENERIC);
        else if((result = cuMemcpy2D(&lumaPlaneParameters)) != CUDA_SUCCESS)
            return error("cuMemcpy2D", result, NV_ENC_ERR_GENERIC);
        else if((result = cuMemcpy2D(&chromaPlaneParameters)) != CUDA_SUCCESS)
            return error("cuMemcpy2D", result, NV_ENC_ERR_GENERIC);
        else if((result = cuvidCtxUnlock(lock, 0)) != CUDA_SUCCESS)
            return error("cuvidCtxUnlock", result, NV_ENC_ERR_GENERIC);
        else if((status = context.hardwareEncoder.NvEncMapInputResource(
                encodeBuffer->stInputBfr.nvRegisteredResource,
                &encodeBuffer->stInputBfr.hInputSurface)) != NV_ENC_SUCCESS)
            return status;
        else
            context.hardwareEncoder.NvEncEncodeFrame(encodeBuffer, NULL, inputFrameType);
            //context.hardwareEncoder.NvEncEncodeFrame(encodeBuffer, NULL, tileWidth, tileHeight, inputFrameType);
    }

    framesEncoded++;

    return NV_ENC_SUCCESS;
}
