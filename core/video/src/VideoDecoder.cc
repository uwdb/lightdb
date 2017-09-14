#include "VideoDecoder.h"
#include "EncodeBuffer.h"
#include <assert.h>

unsigned long tempDecodeSurfaces(const Configuration &configuration, const cudaVideoCodec codec) {
    unsigned long decode_surfaces;

    if ((codec == cudaVideoCodec_H264) ||
        (codec == cudaVideoCodec_H264_SVC) ||
        (codec == cudaVideoCodec_H264_MVC)) {
        // assume worst-case of 20 decode surfaces for H264
        decode_surfaces = 20;
    } else if (codec == cudaVideoCodec_VP9) {
        decode_surfaces = 12;
    } else if (codec == cudaVideoCodec_HEVC) {
        // ref HEVC spec: A.4.1 General tier and level limits
        int MaxLumaPS = 35651584; // currently assuming level 6.2, 8Kx4K
        int MaxDpbPicBuf = 6;
        int PicSizeInSamplesY = configuration.width * configuration.height;
        int MaxDpbSize;
        if (PicSizeInSamplesY <= (MaxLumaPS >> 2))
            MaxDpbSize = MaxDpbPicBuf * 4;
        else if (PicSizeInSamplesY <= (MaxLumaPS >> 1))
            MaxDpbSize = MaxDpbPicBuf * 2;
        else if (PicSizeInSamplesY <= ((3 * MaxLumaPS) >> 2))
            MaxDpbSize = (MaxDpbPicBuf * 4) / 3;
        else
            MaxDpbSize = MaxDpbPicBuf;
        MaxDpbSize = MaxDpbSize < 16 ? MaxDpbSize : 16;
        decode_surfaces = MaxDpbSize + 4;
    } else {
        decode_surfaces = 8;
    }

    return decode_surfaces;
}

CudaDecoder::CudaDecoder(const DecodeConfiguration &configuration, FrameQueue& frame_queue, VideoLock& lock)
    : VideoDecoder(configuration, frame_queue)
       {
           cudaVideoCodec codec = cudaVideoCodec_H264;

           unsigned long output_surfaces = 2;
           cudaVideoChromaFormat chromaFormat = cudaVideoChromaFormat_420;
           unsigned long creationFlags = cudaVideoCreate_PreferCUVID;
           cudaVideoSurfaceFormat outputFormat = cudaVideoSurfaceFormat_NV12;
           cudaVideoDeinterlaceMode deinterlaceMode = cudaVideoDeinterlaceMode_Weave;

/*
    CUVIDEOFORMAT format;
    format.codec = cudaVideoCodec_H264;
    format.chroma_format = cudaVideoChromaFormat_420;
    format.coded_width = configuration.width;
    format.coded_height = configuration.height;

           CUresult oResult;

           if (format.codec != cudaVideoCodec_H264 && format.codec != cudaVideoCodec_HEVC) {
               fprintf(stderr, "The sample only supports H264/HEVC input video!\n");
               exit(-1);
           }

           if (format.chroma_format != cudaVideoChromaFormat_420) {
               fprintf(stderr, "The sample only supports 4:2:0 chroma!\n");
               exit(-1);
           }
*/

           //TODO implicit cast DecodeConfiguration -> CUVIDDECODECREATEINFO
           CUVIDDECODECREATEINFO oVideoDecodeCreateInfo {
                   .ulWidth = configuration.width,
                   .ulHeight = configuration.height,
                   .ulNumDecodeSurfaces = tempDecodeSurfaces(configuration, codec),
                   .CodecType = codec,
                   .ChromaFormat = chromaFormat,
                   .ulCreationFlags = creationFlags,
                   .bitDepthMinus8 = 0,
                   .Reserved1 = {0},
                   .display_area = {
                       .left = 0,
                       .top = 0,
                       .right = static_cast<short>(configuration.width),
                       .bottom = static_cast<const short>(configuration.height),
                   },
                   .OutputFormat = outputFormat,
                   .DeinterlaceMode = deinterlaceMode,  /**< cudaVideoDeinterlaceMode_XXX */
                   .ulTargetWidth = configuration.width,
                   .ulTargetHeight = configuration.height,
                   .ulNumOutputSurfaces = output_surfaces,
                   .vidLock = lock.get(),
                   .target_rect = {0},
                   .Reserved2 = {0}
           };
/*
           memset(&oVideoDecodeCreateInfo, 0, sizeof(CUVIDDECODECREATEINFO));
           //oVideoDecodeCreateInfo.CodecType = format.codec;
           //oVideoDecodeCreateInfo.ulWidth = format.coded_width;
           //oVideoDecodeCreateInfo.ulHeight = format.coded_height;
           oVideoDecodeCreateInfo.ulNumDecodeSurfaces = 8;
           if ((oVideoDecodeCreateInfo.CodecType == cudaVideoCodec_H264) ||
               (oVideoDecodeCreateInfo.CodecType == cudaVideoCodec_H264_SVC) ||
               (oVideoDecodeCreateInfo.CodecType == cudaVideoCodec_H264_MVC)) {
               // assume worst-case of 20 decode surfaces for H264
               oVideoDecodeCreateInfo.ulNumDecodeSurfaces = 20;
           }
           if (oVideoDecodeCreateInfo.CodecType == cudaVideoCodec_VP9)
               oVideoDecodeCreateInfo.ulNumDecodeSurfaces = 12;
           if (oVideoDecodeCreateInfo.CodecType == cudaVideoCodec_HEVC) {
               // ref HEVC spec: A.4.1 General tier and level limits
               int MaxLumaPS = 35651584; // currently assuming level 6.2, 8Kx4K
               int MaxDpbPicBuf = 6;
               int PicSizeInSamplesY = oVideoDecodeCreateInfo.ulWidth * oVideoDecodeCreateInfo.ulHeight;
               int MaxDpbSize;
               if (PicSizeInSamplesY <= (MaxLumaPS >> 2))
                   MaxDpbSize = MaxDpbPicBuf * 4;
               else if (PicSizeInSamplesY <= (MaxLumaPS >> 1))
                   MaxDpbSize = MaxDpbPicBuf * 2;
               else if (PicSizeInSamplesY <= ((3 * MaxLumaPS) >> 2))
                   MaxDpbSize = (MaxDpbPicBuf * 4) / 3;
               else
                   MaxDpbSize = MaxDpbPicBuf;
               MaxDpbSize = MaxDpbSize < 16 ? MaxDpbSize : 16;
               oVideoDecodeCreateInfo.ulNumDecodeSurfaces = MaxDpbSize + 4;
           }
           //oVideoDecodeCreateInfo.ChromaFormat = format.chroma_format;
           //oVideoDecodeCreateInfo.OutputFormat = cudaVideoSurfaceFormat_NV12;
           oVideoDecodeCreateInfo.DeinterlaceMode = cudaVideoDeinterlaceMode_Weave;

           //if (configuration.width <= 0 || configuration.height <= 0) {
           //    oVideoDecodeCreateInfo.ulTargetWidth = format.display_area.right - format.display_area.left;
           //    oVideoDecodeCreateInfo.ulTargetHeight = format.display_area.bottom - format.display_area.top;
           //} else {
           //    oVideoDecodeCreateInfo.ulTargetWidth = configuration.width;
           //    oVideoDecodeCreateInfo.ulTargetHeight = configuration.height;
           //}
           oVideoDecodeCreateInfo.display_area.left = 0;
           oVideoDecodeCreateInfo.display_area.right = oVideoDecodeCreateInfo.ulTargetWidth;
           oVideoDecodeCreateInfo.display_area.top = 0;
           oVideoDecodeCreateInfo.display_area.bottom = oVideoDecodeCreateInfo.ulTargetHeight;

           //oVideoDecodeCreateInfo.ulNumOutputSurfaces = 2;
           //oVideoDecodeCreateInfo.ulCreationFlags = cudaVideoCreate_PreferCUVID;
           //oVideoDecodeCreateInfo.vidLock = lock.get();
*/

           CUresult result;
           if((result = cuvidCreateDecoder(&handle_, &oVideoDecodeCreateInfo)) != CUDA_SUCCESS)
               throw std::runtime_error(std::to_string(result)); //"cuvidCreateDecoder() failed, error code: %d\n"); //TODO , oResult);

           parameters_ = oVideoDecodeCreateInfo;
}
