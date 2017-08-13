#include "VideoDecoder.h"
#include "EncodeBuffer.h"
#include <assert.h>

/*
static const char* getProfileName(int profile)
{
    switch (profile) {
        case 66:    return "Baseline";
        case 77:    return "Main";
        case 88:    return "Extended";
        case 100:   return "High";
        case 110:   return "High 10";
        case 122:   return "High 4:2:2";
        case 244:   return "High 4:4:4";
        case 44:    return "CAVLC 4:4:4";
    }

    return "Unknown Profile";
}*/

static int CUDAAPI HandleVideoData(void *pUserData, CUVIDSOURCEDATAPACKET *pPacket) {
  assert(pUserData);
  CudaDecoder *pDecoder = (CudaDecoder *)pUserData;

  CUresult oResult = cuvidParseVideoData(pDecoder->m_videoParser, pPacket);
  if (oResult != CUDA_SUCCESS) {
    printf("error!\n");
  }

  return 1;
}

/*
static int CUDAAPI HandleVideoSequence(void *pUserData, CUVIDEOFORMAT *pFormat) {
  assert(pUserData);
  CudaDecoder *pDecoder = (CudaDecoder *)pUserData;

  if ((pFormat->codec != pDecoder->m_oVideoDecodeCreateInfo.CodecType) || // codec-type
      (pFormat->coded_width != pDecoder->m_oVideoDecodeCreateInfo.ulWidth) ||
      (pFormat->coded_height != pDecoder->m_oVideoDecodeCreateInfo.ulHeight) ||
      (pFormat->chroma_format != pDecoder->m_oVideoDecodeCreateInfo.ChromaFormat)) {
    fprintf(stderr, "NvTranscoder doesn't deal with dynamic video format changing\n");
    return 0;
  }

  return 1;
}

static int CUDAAPI HandlePictureDecode(void *pUserData, CUVIDPICPARAMS *pPicParams) {
  assert(pUserData);
  CudaDecoder *pDecoder = (CudaDecoder *)pUserData;
  pDecoder->frameQueue.waitUntilFrameAvailable(pPicParams->CurrPicIdx);
  assert(CUDA_SUCCESS == cuvidDecodePicture(pDecoder->handle(), pPicParams));
  return 1;
}

static int CUDAAPI HandlePictureDisplay(void *pUserData, CUVIDPARSERDISPINFO *pPicParams) {
  assert(pUserData);
  CudaDecoder *pDecoder = (CudaDecoder *)pUserData;
  pDecoder->frameQueue.enqueue(pPicParams);
  pDecoder->decodedFrameCount_++;

  return 1;
}
*/

CudaDecoder::CudaDecoder(const EncodeConfig &configuration, FrameQueue& frameQueue, VideoLock& lock)
    : m_videoSource(NULL), m_videoParser(NULL), handle_(NULL), lock(lock), decodedFrameCount_(0),
      complete_(false), frameQueue(frameQueue), configuration(configuration) {
    CUVIDEOFORMAT format;
    format.codec = cudaVideoCodec_H264;
    format.chroma_format = cudaVideoChromaFormat_420;
    format.coded_width = configuration.width;
    format.coded_height = configuration.height;
    InitVideoDecoder(format);
}

CudaDecoder::~CudaDecoder(void) {
    if (handle())
        cuvidDestroyDecoder(handle());
    //Deinitialize();
}

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

void CudaDecoder::InitVideoDecoder(CUVIDEOFORMAT &format) { //const std::string &inputFilename) {
  //assert(configuration.inputFileName);

  CUresult oResult;

/*
  // init video source
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

  // init video decoder
  CUVIDEOFORMAT oFormat;
  cuvidGetSourceVideoFormat(m_videoSource, &oFormat, 0);
*/
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
/*

  // init video parser
  CUVIDPARSERPARAMS oVideoParserParameters;
  memset(&oVideoParserParameters, 0, sizeof(CUVIDPARSERPARAMS));
  oVideoParserParameters.CodecType = oVideoDecodeCreateInfo.CodecType;
  oVideoParserParameters.ulMaxNumDecodeSurfaces = oVideoDecodeCreateInfo.ulNumDecodeSurfaces;
  oVideoParserParameters.ulMaxDisplayDelay = 1;
  oVideoParserParameters.pUserData = this;
  oVideoParserParameters.pfnSequenceCallback = nullptr; //HandleVideoSequence;
  oVideoParserParameters.pfnDecodePicture = nullptr; //HandlePictureDecode;
  oVideoParserParameters.pfnDisplayPicture = nullptr; //HandlePictureDisplay;

  oResult = cuvidCreateVideoParser(&m_videoParser, &oVideoParserParameters);
  if (oResult != CUDA_SUCCESS) {
    fprintf(stderr, "cuvidCreateVideoParser failed, error code: %d\n", oResult);
    exit(-1);
  }*/
 }
/*
void CudaDecoder::Deinitialize() {
    if (handle())
        cuvidDestroyDecoder(handle());

 //  if (m_videoParser)
//      cuvidDestroyVideoParser(m_videoParser);
//  if (m_videoSource)
//      cuvidDestroyVideoSource(m_videoSource);

  handle_ = nullptr;
  //m_videoParser = nullptr;
  //m_videoSource = nullptr;
}
*/
void CudaDecoder::Start() {
  CUresult oResult;

  oResult = cuvidSetVideoSourceState(m_videoSource, cudaVideoState_Started);
  assert(oResult == CUDA_SUCCESS);

  while (cuvidGetVideoSourceState(m_videoSource) == cudaVideoState_Started)
    ;

  complete_ = true;

  frameQueue.endDecode();
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
