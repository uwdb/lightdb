#include "EncodeAPI.h"
#include "EncodeBuffer.h"
#include <gtest/gtest.h>
#include <dlfcn.h>

// Imports from nvUtils.h
#define FALSE 0
#define TRUE  1
#define PRINTERR(message, ...) \
    fprintf(stderr, "%s line %d: " message, __FILE__, __LINE__, ##__VA_ARGS__)
#define FABS(a) ((a) >= 0 ? (a) : -(a))
#define stricmp strcasecmp

NVENCSTATUS EncodeAPI::NvEncOpenEncodeSession(CUcontext context)
{
    return NvEncOpenEncodeSession((void*)context, NV_ENC_DEVICE_TYPE_CUDA);
}

NVENCSTATUS EncodeAPI::NvEncOpenEncodeSession(void* device, uint32_t deviceType)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncOpenEncodeSession(device, deviceType, &encodeSessionHandle);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncOpenEncodeSession";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetEncodeGUIDCount(uint32_t* encodeGUIDCount)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetEncodeGUIDCount(encodeSessionHandle, encodeGUIDCount);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodeGUIDCount";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetEncodeProfileGUIDCount(GUID encodeGUID, uint32_t* encodeProfileGUIDCount)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetEncodeProfileGUIDCount(encodeSessionHandle, encodeGUID, encodeProfileGUIDCount);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodeProfileGUIDCount";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetEncodeProfileGUIDs(GUID encodeGUID, GUID* profileGUIDs, uint32_t guidArraySize, uint32_t* GUIDCount)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetEncodeProfileGUIDs(encodeSessionHandle, encodeGUID, profileGUIDs, guidArraySize, GUIDCount);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "NvEncGetEncodeProfileGUIDs";
        assert(0); // TODO remove all of these asserts
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetEncodeGUIDs(GUID* GUIDs, uint32_t guidArraySize, uint32_t* GUIDCount)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetEncodeGUIDs(encodeSessionHandle, GUIDs, guidArraySize, GUIDCount);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodeGUIDs";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetInputFormatCount(GUID encodeGUID, uint32_t* inputFmtCount)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetInputFormatCount(encodeSessionHandle, encodeGUID, inputFmtCount);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetInputFormatCount";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetInputFormats(GUID encodeGUID, NV_ENC_BUFFER_FORMAT* inputFmts, uint32_t inputFmtArraySize, uint32_t* inputFmtCount)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetInputFormats(encodeSessionHandle, encodeGUID, inputFmts, inputFmtArraySize, inputFmtCount);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetInputFormats";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetEncodeCaps(GUID encodeGUID, NV_ENC_CAPS_PARAM* capsParam, int* capsVal)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetEncodeCaps(encodeSessionHandle, encodeGUID, capsParam, capsVal);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodeCaps";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetEncodePresetCount(GUID encodeGUID, uint32_t* encodePresetGUIDCount)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetEncodePresetCount(encodeSessionHandle, encodeGUID, encodePresetGUIDCount);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodePresetCount";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetEncodePresetGUIDs(GUID encodeGUID, GUID* presetGUIDs, uint32_t guidArraySize, uint32_t* encodePresetGUIDCount)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetEncodePresetGUIDs(encodeSessionHandle, encodeGUID, presetGUIDs, guidArraySize, encodePresetGUIDCount);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodePresetGUIDs";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetEncodePresetConfig(GUID encodeGUID, GUID  presetGUID, NV_ENC_PRESET_CONFIG* presetConfig)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetEncodePresetConfig(encodeSessionHandle, encodeGUID, presetGUID, presetConfig);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodePresetConfig";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncCreateInputBuffer(uint32_t width, uint32_t height, void** inputBuffer, NV_ENC_BUFFER_FORMAT inputFormat)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    NV_ENC_CREATE_INPUT_BUFFER createInputBufferParams;

    memset(&createInputBufferParams, 0, sizeof(createInputBufferParams));
    SET_VER(createInputBufferParams, NV_ENC_CREATE_INPUT_BUFFER);

    createInputBufferParams.width = width;
    createInputBufferParams.height = height;
    createInputBufferParams.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;
    createInputBufferParams.bufferFmt = inputFormat;

    nvStatus = m_pEncodeAPI->nvEncCreateInputBuffer(encodeSessionHandle, &createInputBufferParams);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncCreateInputBuffer";
        assert(0);
    }

    *inputBuffer = createInputBufferParams.inputBuffer;

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncDestroyInputBuffer(NV_ENC_INPUT_PTR inputBuffer)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    if (inputBuffer)
    {
        nvStatus = m_pEncodeAPI->nvEncDestroyInputBuffer(encodeSessionHandle, inputBuffer);
        if (nvStatus != NV_ENC_SUCCESS)
        {
            LOG(ERROR) << "nvEncDestroyInputBuffer";
            assert(0);
        }
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncCreateMVBuffer(uint32_t size, void** bitstreamBuffer)
{
    NVENCSTATUS status;
    NV_ENC_CREATE_MV_BUFFER stAllocMVBuffer;
    memset(&stAllocMVBuffer, 0, sizeof(stAllocMVBuffer));
    SET_VER(stAllocMVBuffer, NV_ENC_CREATE_MV_BUFFER);
    status = m_pEncodeAPI->nvEncCreateMVBuffer(encodeSessionHandle, &stAllocMVBuffer);
    if (status != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncCreateMVBuffer";
        assert(0);
    }
    *bitstreamBuffer = stAllocMVBuffer.mvBuffer;
    return status;
}

NVENCSTATUS EncodeAPI::NvEncDestroyMVBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer)
{
    NVENCSTATUS status;
    NV_ENC_CREATE_MV_BUFFER stAllocMVBuffer;
    memset(&stAllocMVBuffer, 0, sizeof(stAllocMVBuffer));
    SET_VER(stAllocMVBuffer, NV_ENC_CREATE_MV_BUFFER);
    status = m_pEncodeAPI->nvEncDestroyMVBuffer(encodeSessionHandle, bitstreamBuffer);
    if (status != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncDestroyMVBuffer";
        assert(0);
    }
    bitstreamBuffer = NULL;
    return status;
}

NVENCSTATUS EncodeAPI::NvRunMotionEstimationOnly(MotionEstimationBuffer *pMEBuffer, MEOnlyConfig *pMEOnly)
{
    NVENCSTATUS nvStatus;
    NV_ENC_MEONLY_PARAMS stMEOnlyParams;
    SET_VER(stMEOnlyParams,NV_ENC_MEONLY_PARAMS);
    stMEOnlyParams.referenceFrame = pMEBuffer->input_buffer[0].input_surface;
    stMEOnlyParams.inputBuffer = pMEBuffer->input_buffer[1].input_surface;
    stMEOnlyParams.bufferFmt = pMEBuffer->input_buffer[1].buffer_format;
    stMEOnlyParams.inputWidth = pMEBuffer->input_buffer[1].width;
    stMEOnlyParams.inputHeight = pMEBuffer->input_buffer[1].height;
    stMEOnlyParams.mvBuffer = pMEBuffer->output_buffer.bitstreamBuffer;
    stMEOnlyParams.completionEvent = pMEBuffer->output_buffer.outputEvent;
    nvStatus = m_pEncodeAPI->nvEncRunMotionEstimationOnly(encodeSessionHandle, &stMEOnlyParams);
    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncCreateBitstreamBuffer(uint32_t size, void** bitstreamBuffer)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    NV_ENC_CREATE_BITSTREAM_BUFFER createBitstreamBufferParams;

    memset(&createBitstreamBufferParams, 0, sizeof(createBitstreamBufferParams));
    SET_VER(createBitstreamBufferParams, NV_ENC_CREATE_BITSTREAM_BUFFER);

    createBitstreamBufferParams.size = size;
    createBitstreamBufferParams.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;

    nvStatus = m_pEncodeAPI->nvEncCreateBitstreamBuffer(encodeSessionHandle, &createBitstreamBufferParams);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncCreateBitstreamBuffer";
        assert(0);
    }

    *bitstreamBuffer = createBitstreamBufferParams.bitstreamBuffer;

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncDestroyBitstreamBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    if (bitstreamBuffer)
    {
        nvStatus = m_pEncodeAPI->nvEncDestroyBitstreamBuffer(encodeSessionHandle, bitstreamBuffer);
        if (nvStatus != NV_ENC_SUCCESS)
        {
            LOG(ERROR) << "nvEncDestroyBitstreamBuffer";
            assert(0);
        }
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncLockBitstream(NV_ENC_LOCK_BITSTREAM* lockBitstreamBufferParams)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncLockBitstream(encodeSessionHandle, lockBitstreamBufferParams);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncLockBitstream";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncUnlockBitstream(NV_ENC_OUTPUT_PTR bitstreamBuffer)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncUnlockBitstream(encodeSessionHandle, bitstreamBuffer);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncUnlockBitstream";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncLockInputBuffer(void* inputBuffer, void** bufferDataPtr, uint32_t* pitch)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    NV_ENC_LOCK_INPUT_BUFFER lockInputBufferParams;

    memset(&lockInputBufferParams, 0, sizeof(lockInputBufferParams));
    SET_VER(lockInputBufferParams, NV_ENC_LOCK_INPUT_BUFFER);

    lockInputBufferParams.inputBuffer = inputBuffer;
    nvStatus = m_pEncodeAPI->nvEncLockInputBuffer(encodeSessionHandle, &lockInputBufferParams);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncLockInputBuffer";
        assert(0);
    }

    *bufferDataPtr = lockInputBufferParams.bufferDataPtr;
    *pitch = lockInputBufferParams.pitch;

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncUnlockInputBuffer(NV_ENC_INPUT_PTR inputBuffer)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncUnlockInputBuffer(encodeSessionHandle, inputBuffer);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncUnlockInputBuffer";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetEncodeStats(NV_ENC_STAT* encodeStats)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetEncodeStats(encodeSessionHandle, encodeStats);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodeStats";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncGetSequenceParams(NV_ENC_SEQUENCE_PARAM_PAYLOAD* sequenceParamPayload)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncGetSequenceParams(encodeSessionHandle, sequenceParamPayload);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetSequenceParams";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncRegisterAsyncEvent(void** completionEvent)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    NV_ENC_EVENT_PARAMS eventParams;

    memset(&eventParams, 0, sizeof(eventParams));
    SET_VER(eventParams, NV_ENC_EVENT_PARAMS);

#if defined (NV_WINDOWS)
    eventParams.completionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    eventParams.completionEvent = NULL;
#endif
    nvStatus = m_pEncodeAPI->nvEncRegisterAsyncEvent(encodeSessionHandle, &eventParams);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncRegisterAsyncEvent";
        assert(0);
    }

    *completionEvent = eventParams.completionEvent;

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncUnregisterAsyncEvent(void* completionEvent)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    NV_ENC_EVENT_PARAMS eventParams;

    if (completionEvent)
    {
        memset(&eventParams, 0, sizeof(eventParams));
        SET_VER(eventParams, NV_ENC_EVENT_PARAMS);

        eventParams.completionEvent = completionEvent;

        nvStatus = m_pEncodeAPI->nvEncUnregisterAsyncEvent(encodeSessionHandle, &eventParams);
        if (nvStatus != NV_ENC_SUCCESS)
        {
            LOG(ERROR) << "nvEncUnregisterAsyncEvent";
            assert(0);
        }
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncMapInputResource(void* registeredResource, void** mappedResource)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    NV_ENC_MAP_INPUT_RESOURCE mapInputResParams;

    memset(&mapInputResParams, 0, sizeof(mapInputResParams));
    SET_VER(mapInputResParams, NV_ENC_MAP_INPUT_RESOURCE);

    mapInputResParams.registeredResource = registeredResource;

    nvStatus = m_pEncodeAPI->nvEncMapInputResource(encodeSessionHandle, &mapInputResParams);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncMapInputResource";
        assert(0);
    }

    *mappedResource = mapInputResParams.mappedResource;

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncUnmapInputResource(NV_ENC_INPUT_PTR mappedInputBuffer)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    
    if (mappedInputBuffer)
    {
        nvStatus = m_pEncodeAPI->nvEncUnmapInputResource(encodeSessionHandle, mappedInputBuffer);
        if (nvStatus != NV_ENC_SUCCESS)
        {
            LOG(ERROR) << "nvEncUnmapInputResource";
            assert(0);
        }
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncDestroyEncoder()
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    if (m_bEncoderInitialized)
    {
        nvStatus = m_pEncodeAPI->nvEncDestroyEncoder(encodeSessionHandle);

        encodeSessionHandle = nullptr;
        m_bEncoderInitialized = false;
        encoderCreated_ = false;
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::ResetEncoder()
{
    NV_ENC_RECONFIGURE_PARAMS parameters = {};
    parameters.version = NV_ENC_RECONFIGURE_PARAMS_VER;
    parameters.resetEncoder = 1;
    //parameters.forceIDR = 1;
    parameters.reInitEncodeParams = m_stCreateEncodeParams;
    parameters.reInitEncodeParams.encodeConfig->encodeCodecConfig.hevcConfig.repeatSPSPPS = 1;
    parameters.reInitEncodeParams.encodeConfig->encodeCodecConfig.hevcConfig.repeatSPSPPS = 1;

    //parameters.reInitEncodeParams.frameRateNum *= 2;
    //parameters.reInitEncodeParams.frameRateDen *= 2;
    auto result = m_pEncodeAPI->nvEncReconfigureEncoder(encodeSessionHandle, &parameters);
    if(result != NV_ENC_SUCCESS)
        throw GpuEncodeRuntimeError("Call to api.nvEncReconfigureEncoder failed", result);
    return result;
}

NVENCSTATUS EncodeAPI::NvEncInvalidateRefFrames(const NvEncPictureCommand *pEncPicCommand)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    for (uint32_t i = 0; i < pEncPicCommand->numRefFramesToInvalidate; i++)
    {
        nvStatus = m_pEncodeAPI->nvEncInvalidateRefFrames(encodeSessionHandle, pEncPicCommand->refFrameNumbers[i]);
    }

    return nvStatus;
}


NVENCSTATUS EncodeAPI::NvEncOpenEncodeSessionEx(void* device, NV_ENC_DEVICE_TYPE deviceType)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS openSessionExParams;

    if(m_bEncoderInitialized)
        return NV_ENC_SUCCESS;
        //throw "m_bEncoderInitialized"; //TODO

    memset(&openSessionExParams, 0, sizeof(openSessionExParams));
    SET_VER(openSessionExParams, NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS);

    openSessionExParams.device = device;
    openSessionExParams.deviceType = deviceType;
    openSessionExParams.apiVersion = NVENCAPI_VERSION;

    nvStatus = m_pEncodeAPI->nvEncOpenEncodeSessionEx(&openSessionExParams, &encodeSessionHandle);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncOpenEncodeSessionEx";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncRegisterResource(NV_ENC_INPUT_RESOURCE_TYPE resourceType, void* resourceToRegister,
                                             uint32_t width, uint32_t height, uint32_t pitch, void** registeredResource,
                                             NV_ENC_BUFFER_FORMAT format)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    NV_ENC_REGISTER_RESOURCE registerResParams;

    memset(&registerResParams, 0, sizeof(registerResParams));
    SET_VER(registerResParams, NV_ENC_REGISTER_RESOURCE);

    registerResParams.resourceType = resourceType;
    registerResParams.resourceToRegister = resourceToRegister;
    registerResParams.width = width;
    registerResParams.height = height;
    registerResParams.pitch = pitch;
    registerResParams.bufferFormat = format; //NV_ENC_BUFFER_FORMAT_NV12_PL;

    nvStatus = m_pEncodeAPI->nvEncRegisterResource(encodeSessionHandle, &registerResParams);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncRegisterResource";
        assert(0);
    }

    *registeredResource = registerResParams.registeredResource;

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncUnregisterResource(NV_ENC_REGISTERED_PTR registeredRes)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    nvStatus = m_pEncodeAPI->nvEncUnregisterResource(encodeSessionHandle, registeredRes);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        printf("nvEncUnregisterResource error %d %ld\n", nvStatus, reinterpret_cast<unsigned long>(registeredRes));
        LOG(ERROR) << "nvEncUnregisterResource";
        assert(0);
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::NvEncReconfigureEncoder(const NvEncPictureCommand *pEncPicCommand)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    if (pEncPicCommand->bBitrateChangePending || pEncPicCommand->bResolutionChangePending)
    {
        if (pEncPicCommand->bResolutionChangePending)
        {
            m_uCurWidth = pEncPicCommand->newWidth;
            m_uCurHeight = pEncPicCommand->newHeight;
            if ((m_uCurWidth > m_uMaxWidth) || (m_uCurHeight > m_uMaxHeight))
            {
                return NV_ENC_ERR_INVALID_PARAM;
            }
            m_stCreateEncodeParams.encodeWidth = m_uCurWidth;
            m_stCreateEncodeParams.encodeHeight = m_uCurHeight;
            m_stCreateEncodeParams.darWidth = m_uCurWidth;
            m_stCreateEncodeParams.darHeight = m_uCurHeight;
        }

        if (pEncPicCommand->bBitrateChangePending)
        {
            m_stEncodeConfig.rcParams.averageBitRate = pEncPicCommand->newBitrate;
            m_stEncodeConfig.rcParams.maxBitRate = pEncPicCommand->newBitrate;
            m_stEncodeConfig.rcParams.vbvBufferSize = pEncPicCommand->newVBVSize != 0 ? pEncPicCommand->newVBVSize : (pEncPicCommand->newBitrate * m_stCreateEncodeParams.frameRateDen) / m_stCreateEncodeParams.frameRateNum;
            m_stEncodeConfig.rcParams.vbvInitialDelay = m_stEncodeConfig.rcParams.vbvBufferSize;
        }

        NV_ENC_RECONFIGURE_PARAMS stReconfigParams;
        memset(&stReconfigParams, 0, sizeof(stReconfigParams));
        memcpy(&stReconfigParams.reInitEncodeParams, &m_stCreateEncodeParams, sizeof(m_stCreateEncodeParams));
        stReconfigParams.version = NV_ENC_RECONFIGURE_PARAMS_VER;
        stReconfigParams.forceIDR = pEncPicCommand->bResolutionChangePending ? 1 : 0;

        nvStatus = m_pEncodeAPI->nvEncReconfigureEncoder(encodeSessionHandle, &stReconfigParams);
        if (nvStatus != NV_ENC_SUCCESS)
        {
            LOG(ERROR) << "nvEncReconfigureEncoder";
            assert(0);
        }
    }

    return nvStatus;
}

EncodeAPI::EncodeAPI(void* device, NV_ENC_DEVICE_TYPE deviceType)
    : frameEncodedHandler(nullptr), motionEstimationEncodedHandler(nullptr), encodeSessionHandle(nullptr), moved_(false)
{
    NVENCSTATUS status;
    m_bEncoderInitialized = false;
    m_pEncodeAPI = NULL;
    m_hinstLib = NULL;
    //m_fOutput = NULL;
    m_EncodeIdx = 0;
    m_uCurWidth = 0;
    m_uCurHeight = 0;
    m_uMaxWidth = 0;
    m_uMaxHeight = 0;

    memset(&m_stCreateEncodeParams, 0, sizeof(m_stCreateEncodeParams));
    SET_VER(m_stCreateEncodeParams, NV_ENC_INITIALIZE_PARAMS);

    memset(&m_stEncodeConfig, 0, sizeof(m_stEncodeConfig));
    SET_VER(m_stEncodeConfig, NV_ENC_CONFIG);

    if((status = Initialize(device, deviceType)) != NV_ENC_SUCCESS)
        throw GpuEncodeRuntimeError("Call to api.Initialize failed", status);
}

EncodeAPI::EncodeAPI(GPUContext& context)
    : EncodeAPI(context.get())
{ }

EncodeAPI::~EncodeAPI()
{
    if(!moved_)
    {
        //TODO log error on failure
        Deinitialize(); //TODO just remove init/deinit functions

        // clean up encode API resources here
        if (m_pEncodeAPI)
        {
            delete m_pEncodeAPI;
            m_pEncodeAPI = NULL;
        }

        if (m_hinstLib)
        {
#if defined (NV_WINDOWS)
            FreeLibrary(m_hinstLib);
#else
            dlclose(m_hinstLib);
#endif

            m_hinstLib = NULL;
        }
    }
}

NVENCSTATUS EncodeAPI::ValidateEncodeGUID (GUID inputCodecGuid)
{
    unsigned int i, codecFound, encodeGUIDCount, encodeGUIDArraySize;
    NVENCSTATUS nvStatus;
    GUID *encodeGUIDArray;

    nvStatus = m_pEncodeAPI->nvEncGetEncodeGUIDCount(encodeSessionHandle, &encodeGUIDCount);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodeGUIDCount";
        assert(0);
        return nvStatus;
    }

    encodeGUIDArray = new GUID[encodeGUIDCount];
    memset(encodeGUIDArray, 0, sizeof(GUID)* encodeGUIDCount);

    encodeGUIDArraySize = 0;
    nvStatus = m_pEncodeAPI->nvEncGetEncodeGUIDs(encodeSessionHandle, encodeGUIDArray, encodeGUIDCount, &encodeGUIDArraySize);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodeGUIDs";
        delete[] encodeGUIDArray;
        assert(0);
        return nvStatus;
    }

    if(encodeGUIDArraySize > encodeGUIDCount)
        LOG(ERROR) << "encodeGUIDArraySize <= encodeGUIDCount";
    assert(encodeGUIDArraySize <= encodeGUIDCount);

    codecFound = 0;
    for (i = 0; i < encodeGUIDArraySize; i++)
    {
        if (inputCodecGuid == encodeGUIDArray[i])
        {
            codecFound = 1;
            break;
        }
    }

    delete[] encodeGUIDArray;

    if (codecFound)
        return NV_ENC_SUCCESS;
    else
        return NV_ENC_ERR_INVALID_PARAM;
}

NVENCSTATUS EncodeAPI::ValidatePresetGUID(const EncodeConfiguration &configuration)
{
    return ValidatePresetGUID(configuration.preset, configuration.codec);
}

NVENCSTATUS EncodeAPI::ValidatePresetGUID(GUID inputPresetGuid, int codec)
{
    GUID inputCodecGUID = codec == NV_ENC_H264 ? NV_ENC_CODEC_H264_GUID : NV_ENC_CODEC_HEVC_GUID;
    return ValidatePresetGUID(inputPresetGuid, inputCodecGUID);
}

NVENCSTATUS EncodeAPI::ValidatePresetGUID(GUID inputPresetGuid, GUID inputCodecGuid)
{
    uint32_t i, presetFound, presetGUIDCount, presetGUIDArraySize;
    NVENCSTATUS nvStatus;
    GUID *presetGUIDArray;

    nvStatus = m_pEncodeAPI->nvEncGetEncodePresetCount(encodeSessionHandle, inputCodecGuid, &presetGUIDCount);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodePresetCount";
        assert(0);
        return nvStatus;
    }

    presetGUIDArray = new GUID[presetGUIDCount];
    memset(presetGUIDArray, 0, sizeof(GUID)* presetGUIDCount);

    presetGUIDArraySize = 0;
    nvStatus = m_pEncodeAPI->nvEncGetEncodePresetGUIDs(encodeSessionHandle, inputCodecGuid, presetGUIDArray, presetGUIDCount, &presetGUIDArraySize);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "nvEncGetEncodePresetGUIDs";
        assert(0);
        delete[] presetGUIDArray;
        return nvStatus;
    }

    if(presetGUIDArraySize > presetGUIDCount)
        LOG(ERROR) << "presetGUIDArraySize <= presetGUIDCount";
    assert(presetGUIDArraySize <= presetGUIDCount);

    presetFound = 0;
    for (i = 0; i < presetGUIDArraySize; i++)
    {
        if (inputPresetGuid == presetGUIDArray[i])
        {
            presetFound = 1;
            break;
        }
    }

    delete[] presetGUIDArray;

    if (presetFound)
        return NV_ENC_SUCCESS;
    else
        return NV_ENC_ERR_INVALID_PARAM;
}

NVENCSTATUS EncodeAPI::CreateEncoder(const EncodeConfiguration *pEncCfg)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    if(encoderCreated())
        return NV_ENC_SUCCESS;

    if (pEncCfg == NULL)
    {
        return NV_ENC_ERR_INVALID_PARAM;
    }

    m_uCurWidth = pEncCfg->width;
    m_uCurHeight = pEncCfg->height;

    m_uMaxWidth = (pEncCfg->max_width > 0 ? pEncCfg->max_width : pEncCfg->width);
    m_uMaxHeight = (pEncCfg->max_height > 0 ? pEncCfg->max_height : pEncCfg->height);

    if ((m_uCurWidth > m_uMaxWidth) || (m_uCurHeight > m_uMaxHeight)) {
        return NV_ENC_ERR_INVALID_PARAM;
    }

    //m_fOutput = pEncCfg->fOutput;

    // TODO need to remove fOutput from configuration if (!pEncCfg->width || !pEncCfg->height || !m_fOutput)
    if (!pEncCfg->width || !pEncCfg->height)
    {
        return NV_ENC_ERR_INVALID_PARAM;
    }

    if ((pEncCfg->inputFormat == NV_ENC_BUFFER_FORMAT_YUV420_10BIT || pEncCfg->inputFormat == NV_ENC_BUFFER_FORMAT_YUV444_10BIT) && (pEncCfg->codec == NV_ENC_H264))
    {
        PRINTERR("10 bit is not supported with H264 \n");
        return NV_ENC_ERR_INVALID_PARAM;
    }

    GUID inputCodecGUID = pEncCfg->codec == NV_ENC_H264 ? NV_ENC_CODEC_H264_GUID : NV_ENC_CODEC_HEVC_GUID;
    nvStatus = ValidateEncodeGUID(inputCodecGUID);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        PRINTERR("codec not supported \n");
        return nvStatus;
    }

    codecGUID = inputCodecGUID;

    // TODO explicit cast from EncodeConfiguration to NV_ENC_INITIALIZE_PARAMS
    m_stCreateEncodeParams.encodeGUID = inputCodecGUID;
    m_stCreateEncodeParams.presetGUID = pEncCfg->preset;
    m_stCreateEncodeParams.encodeWidth = pEncCfg->width;
    m_stCreateEncodeParams.encodeHeight = pEncCfg->height;

    m_stCreateEncodeParams.darWidth = pEncCfg->width;
    m_stCreateEncodeParams.darHeight = pEncCfg->height;
    m_stCreateEncodeParams.frameRateNum = static_cast<uint32_t>(pEncCfg->framerate.fps());
    m_stCreateEncodeParams.frameRateDen = 1;
    m_stCreateEncodeParams.enableEncodeAsync = 0;

    m_stCreateEncodeParams.enablePTD = 1;
    m_stCreateEncodeParams.reportSliceOffsets = 0;
    m_stCreateEncodeParams.enableSubFrameWrite = 0;
    m_stCreateEncodeParams.encodeConfig = &m_stEncodeConfig;
    m_stCreateEncodeParams.maxEncodeWidth = m_uMaxWidth;
    m_stCreateEncodeParams.maxEncodeHeight = m_uMaxHeight;

    // apply preset
    NV_ENC_PRESET_CONFIG stPresetCfg;
    memset(&stPresetCfg, 0, sizeof(NV_ENC_PRESET_CONFIG));
    SET_VER(stPresetCfg, NV_ENC_PRESET_CONFIG);
    SET_VER(stPresetCfg.presetCfg, NV_ENC_CONFIG);

    nvStatus = m_pEncodeAPI->nvEncGetEncodePresetConfig(encodeSessionHandle, m_stCreateEncodeParams.encodeGUID, m_stCreateEncodeParams.presetGUID, &stPresetCfg);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        PRINTERR("nvEncGetEncodePresetConfig returned failure");
        return nvStatus;
    }
    memcpy(&m_stEncodeConfig, &stPresetCfg.presetCfg, sizeof(NV_ENC_CONFIG));

    m_stEncodeConfig.gopLength = pEncCfg->gopLength;
    m_stEncodeConfig.frameIntervalP = pEncCfg->numB + 1;
    if (pEncCfg->pictureStruct == NV_ENC_PIC_STRUCT_FRAME)
    {
        m_stEncodeConfig.frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE_FRAME;
    }
    else
    {
        m_stEncodeConfig.frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE_FIELD;
    }

    m_stEncodeConfig.mvPrecision = NV_ENC_MV_PRECISION_QUARTER_PEL;

    if (pEncCfg->bitrate || pEncCfg->videoBufferingVerifier.maxBitrate)
    {
        m_stEncodeConfig.rcParams.rateControlMode = (NV_ENC_PARAMS_RC_MODE)pEncCfg->quantization.rateControlMode;
        m_stEncodeConfig.rcParams.averageBitRate = pEncCfg->bitrate;
        m_stEncodeConfig.rcParams.maxBitRate = pEncCfg->videoBufferingVerifier.maxBitrate;
        m_stEncodeConfig.rcParams.vbvBufferSize = pEncCfg->videoBufferingVerifier.size;
        m_stEncodeConfig.rcParams.vbvInitialDelay = pEncCfg->videoBufferingVerifier.size * 9 / 10;
    }
    else
    {
        m_stEncodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CONSTQP;
    }

    if (pEncCfg->quantization.rateControlMode == 0)
    {
        m_stEncodeConfig.rcParams.constQP.qpInterP = pEncCfg->preset == NV_ENC_PRESET_LOSSLESS_HP_GUID? 0 : pEncCfg->quantization.quantizationParameter;
        m_stEncodeConfig.rcParams.constQP.qpInterB = pEncCfg->preset == NV_ENC_PRESET_LOSSLESS_HP_GUID? 0 : pEncCfg->quantization.quantizationParameter;
        m_stEncodeConfig.rcParams.constQP.qpIntra = pEncCfg->preset == NV_ENC_PRESET_LOSSLESS_HP_GUID? 0 : pEncCfg->quantization.quantizationParameter;
    }

    // set up initial QP value
    if (pEncCfg->quantization.rateControlMode == NV_ENC_PARAMS_RC_VBR || pEncCfg->quantization.rateControlMode == NV_ENC_PARAMS_RC_VBR_MINQP ||
        pEncCfg->quantization.rateControlMode == NV_ENC_PARAMS_RC_2_PASS_VBR) {
        m_stEncodeConfig.rcParams.enableInitialRCQP = 1;
        m_stEncodeConfig.rcParams.initialRCQP.qpInterP  = pEncCfg->quantization.quantizationParameter;
        if(pEncCfg->quantization.i_quant_factor != 0.0 && pEncCfg->quantization.b_quant_factor != 0.0) {
            m_stEncodeConfig.rcParams.initialRCQP.qpIntra = (int)(pEncCfg->quantization.quantizationParameter * FABS(pEncCfg->quantization.i_quant_factor) + pEncCfg->quantization.i_quant_offset);
            m_stEncodeConfig.rcParams.initialRCQP.qpInterB = (int)(pEncCfg->quantization.quantizationParameter * FABS(pEncCfg->quantization.b_quant_factor) + pEncCfg->quantization.b_quant_offset);
        } else {
            m_stEncodeConfig.rcParams.initialRCQP.qpIntra = pEncCfg->quantization.quantizationParameter;
            m_stEncodeConfig.rcParams.initialRCQP.qpInterB = pEncCfg->quantization.quantizationParameter;
        }

    }

    if (pEncCfg->inputFormat == NV_ENC_BUFFER_FORMAT_YUV444 || pEncCfg->inputFormat == NV_ENC_BUFFER_FORMAT_YUV444_10BIT)
    {
        if (pEncCfg->codec == NV_ENC_HEVC) {
            m_stEncodeConfig.encodeCodecConfig.hevcConfig.chromaFormatIDC = 3;
        } else {
            m_stEncodeConfig.encodeCodecConfig.h264Config.chromaFormatIDC = 3;
        }
    }
    else
    {
        if (pEncCfg->codec == NV_ENC_HEVC) {
            m_stEncodeConfig.encodeCodecConfig.hevcConfig.chromaFormatIDC = 1;
        } else {
            m_stEncodeConfig.encodeCodecConfig.h264Config.chromaFormatIDC = 1;
        }
    }

    if (pEncCfg->inputFormat == NV_ENC_BUFFER_FORMAT_YUV420_10BIT || pEncCfg->inputFormat == NV_ENC_BUFFER_FORMAT_YUV444_10BIT)
    {
        if (pEncCfg->codec == NV_ENC_HEVC) {
            m_stEncodeConfig.encodeCodecConfig.hevcConfig.pixelBitDepthMinus8 = 2;
        }
    }

    if (pEncCfg->intraRefresh.enabled)
    {
        if (pEncCfg->codec == NV_ENC_HEVC)
        {
            m_stEncodeConfig.encodeCodecConfig.hevcConfig.enableIntraRefresh = 1;
            m_stEncodeConfig.encodeCodecConfig.hevcConfig.intraRefreshPeriod = pEncCfg->intraRefresh.period;
            m_stEncodeConfig.encodeCodecConfig.hevcConfig.intraRefreshCnt = pEncCfg->intraRefresh.duration;
        }
        else
        {
            m_stEncodeConfig.encodeCodecConfig.h264Config.enableIntraRefresh = 1;
            m_stEncodeConfig.encodeCodecConfig.h264Config.intraRefreshPeriod = pEncCfg->intraRefresh.period;
            m_stEncodeConfig.encodeCodecConfig.h264Config.intraRefreshCnt = pEncCfg->intraRefresh.duration;
        }
    }

    if (pEncCfg->flags.enableReferenceFrameInvalidation)
    {
        if (pEncCfg->codec == NV_ENC_HEVC)
        {
            m_stEncodeConfig.encodeCodecConfig.hevcConfig.maxNumRefFramesInDPB = 16;
        }
        else
        {
            m_stEncodeConfig.encodeCodecConfig.h264Config.maxNumRefFrames = 16;
        }
    }

    if (!pEncCfg->quantization.deltaMapFilename.empty())
    {
        m_stEncodeConfig.rcParams.enableExtQPDeltaMap = 1;
    }
    if (pEncCfg->codec == NV_ENC_H264)
    {
        m_stEncodeConfig.encodeCodecConfig.h264Config.idrPeriod = pEncCfg->gopLength;
    }
    else if (pEncCfg->codec == NV_ENC_HEVC)
    {
        m_stEncodeConfig.encodeCodecConfig.hevcConfig.idrPeriod = pEncCfg->gopLength;
    }

    if (pEncCfg->codec == NV_ENC_HEVC)
    {
        m_stEncodeConfig.encodeCodecConfig.hevcConfig.repeatSPSPPS = 1;
    }
    else
    {
        m_stEncodeConfig.encodeCodecConfig.h264Config.repeatSPSPPS = 1;
    }

    NV_ENC_CAPS_PARAM stCapsParam;
    int asyncMode = 0;
    memset(&stCapsParam, 0, sizeof(NV_ENC_CAPS_PARAM));
    SET_VER(stCapsParam, NV_ENC_CAPS_PARAM);

    stCapsParam.capsToQuery = NV_ENC_CAPS_ASYNC_ENCODE_SUPPORT;
    m_pEncodeAPI->nvEncGetEncodeCaps(encodeSessionHandle, m_stCreateEncodeParams.encodeGUID, &stCapsParam, &asyncMode);
    m_stCreateEncodeParams.enableEncodeAsync = asyncMode;

    //pEncCfg->flags.enableAsyncMode = asyncMode;

    if (pEncCfg->flags.enableMEOnly == 1) // || pEncCfg->flags.enableMEOnly == 2)
    {

        stCapsParam.capsToQuery = NV_ENC_CAPS_SUPPORT_MEONLY_MODE;
        m_stCreateEncodeParams.enableMEOnlyMode =  true;
        int meonlyMode = 0;
        nvStatus = m_pEncodeAPI->nvEncGetEncodeCaps(encodeSessionHandle, m_stCreateEncodeParams.encodeGUID, &stCapsParam, &meonlyMode);
        if (nvStatus != NV_ENC_SUCCESS)
        {
            PRINTERR("Encode Session Initialization failed");
            return nvStatus;
        }
        else
        {
            if (meonlyMode == 1)
            {
                printf("NV_ENC_CAPS_SUPPORT_MEONLY_MODE  supported\n");
            }
            else
            {
                PRINTERR("NV_ENC_CAPS_SUPPORT_MEONLY_MODE not supported\n");
                return NV_ENC_ERR_UNSUPPORTED_DEVICE;
            }
        } 
    }

    if (pEncCfg->flags.enableTemporalAQ == 1)
    {
        NV_ENC_CAPS_PARAM stCapsParam;
        memset(&stCapsParam, 0, sizeof(NV_ENC_CAPS_PARAM));
        SET_VER(stCapsParam, NV_ENC_CAPS_PARAM);
        stCapsParam.capsToQuery = NV_ENC_CAPS_SUPPORT_TEMPORAL_AQ;
        int temporalAQSupported = 0;
        nvStatus = m_pEncodeAPI->nvEncGetEncodeCaps(encodeSessionHandle, m_stCreateEncodeParams.encodeGUID, &stCapsParam, &temporalAQSupported);
        if (nvStatus != NV_ENC_SUCCESS)
        {
            PRINTERR("Encode Session Initialization failed");
            return nvStatus;
        }
        else
        {
            if (temporalAQSupported == 1)
            {
                m_stEncodeConfig.rcParams.enableTemporalAQ = 1;
            }
            else
            {
                PRINTERR("NV_ENC_CAPS_SUPPORT_TEMPORAL_AQ not supported\n");
                return NV_ENC_ERR_UNSUPPORTED_DEVICE;
            }
        }
    }

    nvStatus = m_pEncodeAPI->nvEncInitializeEncoder(encodeSessionHandle, &m_stCreateEncodeParams);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        PRINTERR("Encode Session Initialization failed");
        return nvStatus;
    }
    m_bEncoderInitialized = true;
    encoderCreated_ = true;

    return nvStatus;
}

GUID EncodeAPI::GetPresetGUID(const char* encoderPreset, int codec)
{
    GUID presetGUID;

    if (encoderPreset && (stricmp(encoderPreset, "hq") == 0))
    {
        presetGUID = NV_ENC_PRESET_HQ_GUID;
    }
    else if (encoderPreset && (stricmp(encoderPreset, "lowLatencyHP") == 0))
    {
        presetGUID = NV_ENC_PRESET_LOW_LATENCY_HP_GUID;
    }
    else if (encoderPreset && (stricmp(encoderPreset, "hp") == 0))
    {
        presetGUID = NV_ENC_PRESET_HP_GUID;
    }
    else if (encoderPreset && (stricmp(encoderPreset, "lowLatencyHQ") == 0))
    {
        presetGUID = NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;
    }
    else if (encoderPreset && (stricmp(encoderPreset, "lossless") == 0))
    {
        presetGUID = NV_ENC_PRESET_LOSSLESS_HP_GUID;
    }
    else
    {
        if (encoderPreset) {
            LOG(ERROR) << "Unsupported preset guid" << encoderPreset;
            PRINTERR("Unsupported preset guid %s\n", encoderPreset);
            assert(0);
        }
        presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
    }

//    GUID inputCodecGUID = codec == NV_ENC_H264 ? NV_ENC_CODEC_H264_GUID : NV_ENC_CODEC_HEVC_GUID;
//    nvStatus = ValidatePresetGUID(presetGUID, inputCodecGUID);
//    if (nvStatus != NV_ENC_SUCCESS)
//    {
//        presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
//        PRINTERR("Unsupported preset guid %s\n", encoderPreset);
//    }

    return presetGUID;
}

NVENCSTATUS EncodeAPI::ProcessOutput(FILE* output, const EncodeBuffer *pEncodeBuffer)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    if (pEncodeBuffer->output_buffer.bitstreamBuffer == NULL && pEncodeBuffer->output_buffer.EOSFlag == FALSE)
    {
        return NV_ENC_ERR_INVALID_PARAM;
    }

    if (pEncodeBuffer->output_buffer.waitOnEvent == TRUE)
    {
        if (!pEncodeBuffer->output_buffer.outputEvent)
        {
            return NV_ENC_ERR_INVALID_PARAM;
        }
#if defined(NV_WINDOWS)
        WaitForSingleObject(pEncodeBuffer->output_buffer.outputEvent, INFINITE);
#endif
    }

    if (pEncodeBuffer->output_buffer.EOSFlag)
        return NV_ENC_SUCCESS;

    nvStatus = NV_ENC_SUCCESS;
    NV_ENC_LOCK_BITSTREAM lockBitstreamData;
    memset(&lockBitstreamData, 0, sizeof(lockBitstreamData));
    SET_VER(lockBitstreamData, NV_ENC_LOCK_BITSTREAM);
    lockBitstreamData.outputBitstream = pEncodeBuffer->output_buffer.bitstreamBuffer;
    lockBitstreamData.doNotWait = false;

    nvStatus = m_pEncodeAPI->nvEncLockBitstream(encodeSessionHandle, &lockBitstreamData);
    if (nvStatus == NV_ENC_SUCCESS)
    {
        fwrite(lockBitstreamData.bitstreamBufferPtr, 1, lockBitstreamData.bitstreamSizeInBytes, output);
        nvStatus = m_pEncodeAPI->nvEncUnlockBitstream(encodeSessionHandle, pEncodeBuffer->output_buffer.bitstreamBuffer);
    }
    else
    {
        PRINTERR("lock bitstream function failed \n");
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::ProcessMVOutput(FILE* output, const MotionEstimationBuffer *pMEBuffer)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;

    if (pMEBuffer->output_buffer.bitstreamBuffer == NULL && pMEBuffer->output_buffer.EOSFlag == FALSE)
    {
        return NV_ENC_ERR_INVALID_PARAM;
    }

    if (pMEBuffer->output_buffer.waitOnEvent == TRUE)
    {
        if (!pMEBuffer->output_buffer.outputEvent)
        {
            return NV_ENC_ERR_INVALID_PARAM;
        }
#if defined(NV_WINDOWS)
        WaitForSingleObject(pMEBuffer->output_buffer.outputEvent, INFINITE);
#endif
    }

    if (pMEBuffer->output_buffer.EOSFlag)
        return NV_ENC_SUCCESS;

    nvStatus = NV_ENC_SUCCESS;
    NV_ENC_LOCK_BITSTREAM lockBitstreamData;
    memset(&lockBitstreamData, 0, sizeof(lockBitstreamData));
    SET_VER(lockBitstreamData, NV_ENC_LOCK_BITSTREAM);
    lockBitstreamData.outputBitstream = pMEBuffer->output_buffer.bitstreamBuffer;
    lockBitstreamData.doNotWait = 0;

    nvStatus = m_pEncodeAPI->nvEncLockBitstream(encodeSessionHandle, &lockBitstreamData);
    if (nvStatus == NV_ENC_SUCCESS)
    {
        if (codecGUID == NV_ENC_CODEC_H264_GUID)
        {
            unsigned int numMBs = ((m_uMaxWidth + 15) >> 4) * ((m_uMaxHeight + 15) >> 4);
            fprintf(output, "Motion Vectors for input frame = %d, reference frame = %d\n", pMEBuffer->inputFrameIndex, pMEBuffer->referenceFrameIndex);
            fprintf(output, "block, mb_type, partitionType, "
                "MV[0].x, MV[0].y, MV[1].x, MV[1].y, MV[2].x, MV[2].y, MV[3].x, MV[3].y, cost\n");
            NV_ENC_H264_MV_DATA *outputMV = (NV_ENC_H264_MV_DATA *)lockBitstreamData.bitstreamBufferPtr;
            for (unsigned int i = 0; i < numMBs; i++)
            {
                fprintf(output, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", \
                    i, outputMV[i].mbType, outputMV[i].partitionType, \
                    outputMV[i].mv[0].mvx, outputMV[i].mv[0].mvy, outputMV[i].mv[1].mvx, outputMV[i].mv[1].mvy, \
                    outputMV[i].mv[2].mvx, outputMV[i].mv[2].mvy, outputMV[i].mv[3].mvx, outputMV[i].mv[3].mvy, outputMV[i].mbCost);
            }
            fprintf(output, "\n");
        }
        else
        {
            unsigned int numCTBs = ((m_uMaxWidth + 31) >> 5) * ((m_uMaxHeight + 31) >> 5);
            fprintf(output, "Motion Vectors for input frame = %d, reference frame = %d\n", pMEBuffer->inputFrameIndex, pMEBuffer->referenceFrameIndex);
            NV_ENC_HEVC_MV_DATA *outputMV = (NV_ENC_HEVC_MV_DATA *)lockBitstreamData.bitstreamBufferPtr;
            fprintf(output, "ctb, cuType, cuSize, partitionMode, "
                "MV[0].x, MV[0].y, MV[1].x, MV[1].y, MV[2].x, MV[2].y, MV[3].x, MV[3].y\n");
            bool lastCUInCTB = false;
            for (unsigned int i = 0; i < numCTBs; i++)
            {
                do
                {
                    lastCUInCTB = outputMV->lastCUInCTB ? true : false;
                    fprintf(output, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", \
                        i, outputMV->cuType, outputMV->cuSize, outputMV->partitionMode, \
                        outputMV->mv[0].mvx, outputMV->mv[0].mvy, outputMV->mv[1].mvx, outputMV->mv[1].mvy, \
                        outputMV->mv[2].mvx, outputMV->mv[2].mvy, outputMV->mv[3].mvx, outputMV->mv[3].mvy);
                    outputMV += 1;
                } while (!lastCUInCTB);
            }
            fprintf(output, "\n");
        }
        nvStatus = m_pEncodeAPI->nvEncUnlockBitstream(encodeSessionHandle, pMEBuffer->output_buffer.bitstreamBuffer);
    }
    else
    {
        PRINTERR("lock bitstream function failed \n");
    }

    return nvStatus;
}

NVENCSTATUS EncodeAPI::Initialize(void* device, NV_ENC_DEVICE_TYPE deviceType)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    MYPROC nvEncodeAPICreateInstance; // function pointer to create instance in nvEncodeAPI

    if(m_bEncoderInitialized)
        LOG(ERROR) << "!m_bEncoderInitialized";
    assert(!m_bEncoderInitialized);

#if defined(NV_WINDOWS)
#if defined (_WIN64)
    m_hinstLib = LoadLibrary(TEXT("nvEncodeAPI64.dll"));
#else
    m_hinstLib = LoadLibrary(TEXT("nvEncodeAPI.dll"));
#endif
#else
    m_hinstLib = dlopen("libnvidia-encode.so.1", RTLD_LAZY);
#endif
    if (m_hinstLib == NULL)
        return NV_ENC_ERR_OUT_OF_MEMORY;

#if defined(NV_WINDOWS)
    nvEncodeAPICreateInstance = (MYPROC)GetProcAddress(m_hinstLib, "NvEncodeAPICreateInstance");
#else
    nvEncodeAPICreateInstance = (MYPROC)dlsym(m_hinstLib, "NvEncodeAPICreateInstance");
#endif

    if (nvEncodeAPICreateInstance == NULL)
        return NV_ENC_ERR_OUT_OF_MEMORY;

    m_pEncodeAPI = new NV_ENCODE_API_FUNCTION_LIST;
    if (m_pEncodeAPI == NULL)
        return NV_ENC_ERR_OUT_OF_MEMORY;

    memset(m_pEncodeAPI, 0, sizeof(NV_ENCODE_API_FUNCTION_LIST));
    m_pEncodeAPI->version = NV_ENCODE_API_FUNCTION_LIST_VER;
    nvStatus = nvEncodeAPICreateInstance(m_pEncodeAPI);
    if (nvStatus != NV_ENC_SUCCESS)
        return nvStatus;

    nvStatus = NvEncOpenEncodeSessionEx(device, deviceType);
    if (nvStatus != NV_ENC_SUCCESS)
        return nvStatus;

    m_bEncoderInitialized = true;

    return NV_ENC_SUCCESS;
}

NVENCSTATUS EncodeAPI::NvEncEncodeFrame(EncodeBuffer *pEncodeBuffer, NvEncPictureCommand *encPicCommand,
                                           NV_ENC_PIC_STRUCT ePicStruct,
                                           bool isFirstFrame, int8_t *qpDeltaMapArray, uint32_t qpDeltaMapArraySize)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    NV_ENC_PIC_PARAMS encPicParams;

    memset(&encPicParams, 0, sizeof(encPicParams));
    SET_VER(encPicParams, NV_ENC_PIC_PARAMS);


    encPicParams.inputBuffer = pEncodeBuffer->input_buffer.input_surface;
    encPicParams.bufferFmt = pEncodeBuffer->input_buffer.buffer_format;
    encPicParams.inputWidth = pEncodeBuffer->encoder.configuration().width;
    encPicParams.inputHeight = pEncodeBuffer->encoder.configuration().height;
    encPicParams.outputBitstream = pEncodeBuffer->output_buffer.bitstreamBuffer;
    encPicParams.completionEvent = pEncodeBuffer->output_buffer.outputEvent;
    encPicParams.inputTimeStamp = m_EncodeIdx; //TODO think about this, should be a sane PTS
    encPicParams.pictureStruct = ePicStruct;
    encPicParams.qpDeltaMap = qpDeltaMapArray;
    encPicParams.qpDeltaMapSize = qpDeltaMapArraySize;

    // Can we substitute setting intraRefreshEnableFlag to true for this?
    if(isFirstFrame) {
        //ResetEncoder();
        encPicParams.encodePicFlags |= NV_ENC_PIC_FLAG_OUTPUT_SPSPPS | NV_ENC_PIC_FLAG_FORCEIDR;
        //encPicParams.pictureType = NV_ENC_PIC_TYPE_IDR;
        //encPicParams.codecPicParams.hevcPicParams.dis
        //encPicParams.codecPicParams.hevcPicParams.forceIntraRefreshWithFrameCnt = 30;
    }

    if (encPicCommand)
    {
        if (encPicCommand->bForceIDR)
        {
            encPicParams.encodePicFlags |= NV_ENC_PIC_FLAG_FORCEIDR;
        }

        if (encPicCommand->bForceIntraRefresh)
        {
            if (codecGUID == NV_ENC_CODEC_HEVC_GUID)
            {
                encPicParams.codecPicParams.hevcPicParams.forceIntraRefreshWithFrameCnt = encPicCommand->intraRefreshDuration;
            }
            else
            {
                encPicParams.codecPicParams.h264PicParams.forceIntraRefreshWithFrameCnt = encPicCommand->intraRefreshDuration;
            }
        }
    }

    nvStatus = m_pEncodeAPI->nvEncEncodePicture(encodeSessionHandle, &encPicParams);
    if (nvStatus != NV_ENC_SUCCESS && nvStatus != NV_ENC_ERR_NEED_MORE_INPUT)
    {
        LOG(ERROR) << "nvEncEncodePicture";
        printf("nvstatus: %d\n", nvStatus);
        assert(0);
        return nvStatus;
    }

    m_EncodeIdx++;

    return NV_ENC_SUCCESS;
}

NVENCSTATUS EncodeAPI::NvEncFlushEncoderQueue(void *hEOSEvent)
{
    NVENCSTATUS nvStatus = NV_ENC_SUCCESS;
    NV_ENC_PIC_PARAMS encPicParams;
    memset(&encPicParams, 0, sizeof(encPicParams));
    SET_VER(encPicParams, NV_ENC_PIC_PARAMS);
    encPicParams.encodePicFlags = NV_ENC_PIC_FLAG_EOS;
    encPicParams.completionEvent = hEOSEvent;
    if(!m_bEncoderInitialized)
        throw GpuEncodeRuntimeError("Encoder not initialized, but attempting to flush", NV_ENC_ERR_INVALID_DEVICE);

    nvStatus = m_pEncodeAPI->nvEncEncodePicture(encodeSessionHandle, &encPicParams);
    if (nvStatus != NV_ENC_SUCCESS)
    {
        LOG(ERROR) << "NvEncFlushEncoderQueue.nvEncEncodePicture";
        assert(0);
    }
    return nvStatus;
}
