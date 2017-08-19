#include "VideoDecoder.h"
#include "EncodeBuffer.h"
#include <assert.h>

/*
static int CUDAAPI HandleVideoData(void *pUserData, CUVIDSOURCEDATAPACKET *pPacket) {
  assert(pUserData);
  CudaDecoder *pDecoder = (CudaDecoder *)pUserData;

  CUresult oResult = cuvidParseVideoData(pDecoder->m_videoParser, pPacket);
  if (oResult != CUDA_SUCCESS) {
    printf("error!\n");
  }

  return 1;
}*/

CudaDecoder::CudaDecoder(const EncodeConfig &configuration, FrameQueue& frame_queue, VideoLock& lock)
    : VideoDecoder(configuration, frame_queue), m_videoSource(NULL), m_videoParser(NULL),
      decodedFrameCount_(0), handle_(NULL), complete_(false), lock(lock)
       {
    CUVIDEOFORMAT format;
    format.codec = cudaVideoCodec_H264;
    format.chroma_format = cudaVideoChromaFormat_420;
    format.coded_width = configuration.width;
    format.coded_height = configuration.height;

    //InitVideoDecoder(format);

           CUresult oResult;

           if (format.codec != cudaVideoCodec_H264 && format.codec != cudaVideoCodec_HEVC) {
               fprintf(stderr, "The sample only supports H264/HEVC input video!\n");
               exit(-1);
           }

           if (format.chroma_format != cudaVideoChromaFormat_420) {
               fprintf(stderr, "The sample only supports 4:2:0 chroma!\n");
               exit(-1);
           }

           CUVIDDECODECREATEINFO oVideoDecodeCreateInfo;
           memset(&oVideoDecodeCreateInfo, 0, sizeof(CUVIDDECODECREATEINFO));
           oVideoDecodeCreateInfo.CodecType = format.codec;
           oVideoDecodeCreateInfo.ulWidth = format.coded_width;
           oVideoDecodeCreateInfo.ulHeight = format.coded_height;
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
           oVideoDecodeCreateInfo.ChromaFormat = format.chroma_format;
           oVideoDecodeCreateInfo.OutputFormat = cudaVideoSurfaceFormat_NV12;
           oVideoDecodeCreateInfo.DeinterlaceMode = cudaVideoDeinterlaceMode_Weave;

           if (configuration.width <= 0 || configuration.height <= 0) {
               oVideoDecodeCreateInfo.ulTargetWidth = format.display_area.right - format.display_area.left;
               oVideoDecodeCreateInfo.ulTargetHeight = format.display_area.bottom - format.display_area.top;
           } else {
               oVideoDecodeCreateInfo.ulTargetWidth = configuration.width;
               oVideoDecodeCreateInfo.ulTargetHeight = configuration.height;
           }
           oVideoDecodeCreateInfo.display_area.left = 0;
           oVideoDecodeCreateInfo.display_area.right = oVideoDecodeCreateInfo.ulTargetWidth;
           oVideoDecodeCreateInfo.display_area.top = 0;
           oVideoDecodeCreateInfo.display_area.bottom = oVideoDecodeCreateInfo.ulTargetHeight;

           oVideoDecodeCreateInfo.ulNumOutputSurfaces = 2;
           oVideoDecodeCreateInfo.ulCreationFlags = cudaVideoCreate_PreferCUVID;
           oVideoDecodeCreateInfo.vidLock = lock.get();

           oResult = cuvidCreateDecoder(&handle_, &oVideoDecodeCreateInfo);
           if (oResult != CUDA_SUCCESS) {
               fprintf(stderr, "cuvidCreateDecoder() failed, error code: %d\n", oResult);
               exit(-1);
           }

           m_oVideoDecodeCreateInfo = oVideoDecodeCreateInfo;
}

CudaDecoder::~CudaDecoder(void) {
    if (handle())
        cuvidDestroyDecoder(handle());
}

/*
//TODO remove
void CudaDecoder::InitVideoDecoder(const std::string &inputFilename) {
    CUresult oResult;

    CUVIDSOURCEPARAMS oVideoSourceParameters;
    memset(&oVideoSourceParameters, 0, sizeof(CUVIDSOURCEPARAMS));
    oVideoSourceParameters.pUserData = this;
    oVideoSourceParameters.pfnVideoDataHandler = HandleVideoData;
    oVideoSourceParameters.pfnAudioDataHandler = NULL;

    oResult = cuvidCreateVideoSource(&m_videoSource, inputFilename.c_str(), &oVideoSourceParameters);
    if (oResult != CUDA_SUCCESS) {
        fprintf(stderr, "cuvidCreateVideoSource failed\n");
        fprintf(stderr, "Please check if the path exists, or the video is a valid H264 file\n");
        exit(-1);
    }

    CUVIDEOFORMAT oFormat;
    cuvidGetSourceVideoFormat(m_videoSource, &oFormat, 0);

    cuvidDestroyVideoSource(m_videoSource);

    InitVideoDecoder(oFormat);
}

void CudaDecoder::InitVideoDecoder(CUVIDEOFORMAT &format) {
 }
*/

void CudaDecoder::Start() {
  CUresult oResult;

  oResult = cuvidSetVideoSourceState(m_videoSource, cudaVideoState_Started);
  assert(oResult == CUDA_SUCCESS);

  while (cuvidGetVideoSourceState(m_videoSource) == cudaVideoState_Started)
    ;

  complete_ = true;

  frame_queue().endDecode();
}

void CudaDecoder::GetCodecParam(int *width, int *height, int *frame_rate_num, int *frame_rate_den,
                                int *is_progressive) {
  assert(width != NULL && height != NULL && frame_rate_num != NULL && frame_rate_den != NULL);
  CUVIDEOFORMAT oFormat;
  cuvidGetSourceVideoFormat(m_videoSource, &oFormat, 0);

  *width = oFormat.display_area.right - oFormat.display_area.left;
  *height = oFormat.display_area.bottom - oFormat.display_area.top;
  *frame_rate_num = oFormat.frame_rate.numerator;
  *frame_rate_den = oFormat.frame_rate.denominator;
  *is_progressive = oFormat.progressive_sequence;
}
