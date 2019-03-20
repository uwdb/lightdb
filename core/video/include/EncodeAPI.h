#ifndef LIGHTDB_ENCODE_API
#define LIGHTDB_ENCODE_API

#include "ffmpeg/nvEncodeAPI.h"
#include "ffmpeg/nvUtils.h"
#include <cuda.h>
#include <cassert>
#include <functional>

class GPUContext;
struct EncodeConfiguration;
struct EncodeBuffer;
struct MotionEstimationBuffer;

#define SET_VER(configStruct, type) {configStruct.version = type##_VER;}

#if defined (NV_WINDOWS)
    #include "d3d9.h"
    #define NVENCAPI __stdcall
    #pragma warning(disable : 4996)
#elif defined (NV_UNIX)
    #include <dlfcn.h>
    #include <string.h>
    #define NVENCAPI
#endif

#define DEFAULT_I_QFACTOR -0.8f
#define DEFAULT_B_QFACTOR 1.25f
#define DEFAULT_I_QOFFSET 0.f
#define DEFAULT_B_QOFFSET 1.25f


typedef struct _NvEncPictureCommand
{
    bool bResolutionChangePending;
    bool bBitrateChangePending;
    bool bForceIDR;
    bool bForceIntraRefresh;
    bool bInvalidateRefFrames;

    uint32_t newWidth;
    uint32_t newHeight;

    uint32_t newBitrate;
    uint32_t newVBVSize;

    uint32_t  intraRefreshDuration;

    uint32_t  numRefFramesToInvalidate;
    uint32_t  refFrameNumbers[16];
}NvEncPictureCommand;

enum EncodeCodec
{
    NV_ENC_H264 = 0,
    NV_ENC_HEVC = 1,
};

struct MEOnlyConfig
{
    unsigned char *yuv[2][3];
    unsigned int stride[3];
    unsigned int width;
    unsigned int height;
    unsigned int inputFrameIndex;
    unsigned int referenceFrameIndex;
};

typedef void EncodeSessionHandle;
typedef std::function<NVENCSTATUS(const EncodeBuffer&) > FrameEncodedHandler;
typedef std::function<NVENCSTATUS(const MotionEstimationBuffer&) > MotionEstimationEncodedHandler;

class EncodeAPI
{
public:
    uint32_t                                             m_EncodeIdx;
    //FILE                                                *m_fOutput;
    uint32_t                                             m_uMaxWidth;
    uint32_t                                             m_uMaxHeight;
    uint32_t                                             m_uCurWidth;
    uint32_t                                             m_uCurHeight;

protected:
    bool                                                 m_bEncoderInitialized;
    bool                                                 encoderCreated_ = false;
    GUID                                                 codecGUID;

    NV_ENCODE_API_FUNCTION_LIST*                         m_pEncodeAPI;
    HINSTANCE                                            m_hinstLib;
    FrameEncodedHandler                                  *frameEncodedHandler;
    MotionEstimationEncodedHandler                       *motionEstimationEncodedHandler;
    EncodeSessionHandle                                  *encodeSessionHandle;
    NV_ENC_INITIALIZE_PARAMS                             m_stCreateEncodeParams;
    NV_ENC_CONFIG                                        m_stEncodeConfig;
    bool                                                 moved_;

public:
    bool encoderCreated() const { return encoderCreated_; }

    NVENCSTATUS NvEncOpenEncodeSession(CUcontext context);
    NVENCSTATUS NvEncOpenEncodeSession(void* device, uint32_t deviceType);
    NVENCSTATUS NvEncGetEncodeGUIDCount(uint32_t* encodeGUIDCount);
    NVENCSTATUS NvEncGetEncodeProfileGUIDCount(GUID encodeGUID, uint32_t* encodeProfileGUIDCount);
    NVENCSTATUS NvEncGetEncodeProfileGUIDs(GUID encodeGUID, GUID* profileGUIDs, uint32_t guidArraySize, uint32_t* GUIDCount);
    NVENCSTATUS NvEncGetEncodeGUIDs(GUID* GUIDs, uint32_t guidArraySize, uint32_t* GUIDCount);
    NVENCSTATUS NvEncGetInputFormatCount(GUID encodeGUID, uint32_t* inputFmtCount);
    NVENCSTATUS NvEncGetInputFormats(GUID encodeGUID, NV_ENC_BUFFER_FORMAT* inputFmts, uint32_t inputFmtArraySize, uint32_t* inputFmtCount);
    NVENCSTATUS NvEncGetEncodeCaps(GUID encodeGUID, NV_ENC_CAPS_PARAM* capsParam, int* capsVal);
    NVENCSTATUS NvEncGetEncodePresetCount(GUID encodeGUID, uint32_t* encodePresetGUIDCount);
    NVENCSTATUS NvEncGetEncodePresetGUIDs(GUID encodeGUID, GUID* presetGUIDs, uint32_t guidArraySize, uint32_t* encodePresetGUIDCount);
    NVENCSTATUS NvEncGetEncodePresetConfig(GUID encodeGUID, GUID  presetGUID, NV_ENC_PRESET_CONFIG* presetConfig);
    NVENCSTATUS NvEncCreateInputBuffer(uint32_t width, uint32_t height, void** inputBuffer, NV_ENC_BUFFER_FORMAT inputFormat);
    NVENCSTATUS NvEncDestroyInputBuffer(NV_ENC_INPUT_PTR inputBuffer);
    NVENCSTATUS NvEncCreateBitstreamBuffer(uint32_t size, void** bitstreamBuffer);
    NVENCSTATUS NvEncDestroyBitstreamBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer);
    NVENCSTATUS NvEncCreateMVBuffer(uint32_t size, void** bitstreamBuffer);
    NVENCSTATUS NvEncDestroyMVBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer);
    NVENCSTATUS NvRunMotionEstimationOnly(MotionEstimationBuffer *pMEBuffer, MEOnlyConfig *pMEOnly);
    NVENCSTATUS NvEncLockBitstream(NV_ENC_LOCK_BITSTREAM* lockBitstreamBufferParams);
    NVENCSTATUS NvEncUnlockBitstream(NV_ENC_OUTPUT_PTR bitstreamBuffer);
    NVENCSTATUS NvEncLockInputBuffer(void* inputBuffer, void** bufferDataPtr, uint32_t* pitch);
    NVENCSTATUS NvEncUnlockInputBuffer(NV_ENC_INPUT_PTR inputBuffer);
    NVENCSTATUS NvEncGetEncodeStats(NV_ENC_STAT* encodeStats);
    NVENCSTATUS NvEncGetSequenceParams(NV_ENC_SEQUENCE_PARAM_PAYLOAD* sequenceParamPayload);
    NVENCSTATUS NvEncRegisterAsyncEvent(void** completionEvent);
    NVENCSTATUS NvEncUnregisterAsyncEvent(void* completionEvent);
    NVENCSTATUS NvEncMapInputResource(void* registeredResource, void** mappedResource);
    NVENCSTATUS NvEncUnmapInputResource(NV_ENC_INPUT_PTR mappedInputBuffer);
    NVENCSTATUS NvEncDestroyEncoder();
    NVENCSTATUS NvEncInvalidateRefFrames(const NvEncPictureCommand *pEncPicCommand);
    NVENCSTATUS NvEncOpenEncodeSessionEx(void* device, NV_ENC_DEVICE_TYPE deviceType);
    NVENCSTATUS NvEncRegisterResource(NV_ENC_INPUT_RESOURCE_TYPE resourceType, void* resourceToRegister, uint32_t width, uint32_t height, uint32_t pitch, void** registeredResource);
    NVENCSTATUS NvEncUnregisterResource(NV_ENC_REGISTERED_PTR registeredRes);
    NVENCSTATUS NvEncReconfigureEncoder(const NvEncPictureCommand *pEncPicCommand);
    NVENCSTATUS NvEncFlushEncoderQueue(void *hEOSEvent);
    NVENCSTATUS ResetEncoder();

    explicit EncodeAPI(GPUContext& context);
    explicit EncodeAPI(CUcontext context) : EncodeAPI(context, NV_ENC_DEVICE_TYPE_CUDA) { }
    EncodeAPI(void* device, NV_ENC_DEVICE_TYPE deviceType);
    EncodeAPI(const EncodeAPI&) = delete;
    EncodeAPI(EncodeAPI &&other) noexcept
            : m_EncodeIdx(other.m_EncodeIdx),
              m_uMaxWidth(other.m_uMaxWidth),
              m_uMaxHeight(other.m_uMaxHeight),
              m_uCurWidth(other.m_uCurWidth),
              m_uCurHeight(other.m_uCurHeight),
              m_bEncoderInitialized(other.m_bEncoderInitialized),
              encoderCreated_(other.encoderCreated_),
              codecGUID(other.codecGUID),
              m_pEncodeAPI(other.m_pEncodeAPI),
              m_hinstLib(other.m_hinstLib),
              frameEncodedHandler(other.frameEncodedHandler),
              motionEstimationEncodedHandler(other.motionEstimationEncodedHandler),
              encodeSessionHandle(other.encodeSessionHandle),
              m_stCreateEncodeParams(other.m_stCreateEncodeParams),
              m_stEncodeConfig(other.m_stEncodeConfig),
              moved_(false)
    { other.moved_ = true; }


    virtual ~EncodeAPI();
    NVENCSTATUS                                          CreateEncoder(const EncodeConfiguration *pEncCfg);
    NVENCSTATUS                                          NvEncEncodeFrame(EncodeBuffer *pEncodeBuffer, NvEncPictureCommand *encPicCommand,
                                                                          NV_ENC_PIC_STRUCT ePicStruct = NV_ENC_PIC_STRUCT_FRAME,
                                                                          bool isFirstFrame=false, int8_t *qpDeltaMapArray = NULL, uint32_t qpDeltaMapArraySize = 0);
    static GUID                                          GetPresetGUID(const char* encoderPreset, int codec);
    NVENCSTATUS                                          ProcessOutput(FILE* output, const EncodeBuffer *pEncodeBuffer);
    NVENCSTATUS                                          ProcessMVOutput(FILE* output, const MotionEstimationBuffer *pEncodeBuffer);

    NVENCSTATUS                                          ValidatePresetGUID(const EncodeConfiguration&);
    NVENCSTATUS                                          ValidatePresetGUID(GUID inputPresetGuid, int codec);

protected:
    //TODO these two functions should just be removed entirely
    NVENCSTATUS                                          Initialize(void* device, NV_ENC_DEVICE_TYPE deviceType);
    NVENCSTATUS                                          Deinitialize() { return NvEncDestroyEncoder(); }
    NVENCSTATUS                                          ValidateEncodeGUID(GUID inputCodecGuid);
    NVENCSTATUS                                          ValidatePresetGUID(GUID presetCodecGuid, GUID inputCodecGuid);
};

typedef NVENCSTATUS (NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*);

#endif // LIGHTDB_ENCODE_H