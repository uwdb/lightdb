#include <time.h>
#include <pthread.h>

#include <stdio.h>
#include "dynlink_cuda.h"    // <cuda.h>

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include <climits>

#include "Transcoder.h"

#include <ctime>


void* DecodeProc(void *arg)
{
    auto* decoder = static_cast<CudaDecoder*>(arg);
	decoder->Start();

    return nullptr;
}

int MatchFPS(const float fpsRatio, int decodedFrames, int encodedFrames)
{
    if (fpsRatio < 1.f) {
        // need to drop frame
        if (decodedFrames * fpsRatio < (encodedFrames + 1)) {
            return -1;
        }
    }
    else if (fpsRatio > 1.f) {
        // need to duplicate frame	 
        auto duplicate = 0;
        while (decodedFrames*fpsRatio > encodedFrames + duplicate + 1) {
            duplicate++;
        }

        return duplicate;
    }

    return 0;
}

Transcoder::Transcoder(unsigned int height, unsigned int width,
					   unsigned int codec, std::string preset, unsigned int fps, unsigned int gop_length,
	                   unsigned long bitrate, unsigned int rcmode, unsigned int deviceId)
	: preset(preset), encoder(nullptr), frameQueue(nullptr), configuration{0}
{
	outputFilename.reserve(1024);

	configuration.height = height;
	configuration.width = width;
	configuration.endFrameIdx = INT_MAX;
	configuration.bitrate = bitrate;
	configuration.rcMode = rcmode;
	configuration.gopLength = gop_length;
	configuration.codec = codec;
	configuration.fps = fps;
	configuration.qp = 28;
	configuration.i_quant_factor = DEFAULT_I_QFACTOR;
	configuration.b_quant_factor = DEFAULT_B_QFACTOR;
	configuration.i_quant_offset = DEFAULT_I_QOFFSET;
	configuration.b_quant_offset = DEFAULT_B_QOFFSET;
	configuration.presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
	configuration.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
	configuration.encoderPreset = const_cast<char*>(this->preset.c_str()); //TODO
	configuration.deviceID = deviceId;

	//configuration.inputFileName = const_cast<char*>(this->inputFilename.c_str()); //TODO
	//configuration.outputFileName = const_cast<char*>(this->outputFilename.c_str()); //TODO
}

int Transcoder::initialize()
{
	CUresult result;
	NVENCSTATUS nvStatus;
	typedef void *CUDADRIVER;
	CUDADRIVER driver = nullptr;
	CUdevice device;

	if ((result = cuInit(0, __CUDA_API_VERSION, driver)) != CUDA_SUCCESS)
		return 1;
	else if ((result = cuvidInit(0)) != CUDA_SUCCESS)
		return 2;
	else if ((result = cuDeviceGet(&device, configuration.deviceID)) != CUDA_SUCCESS)
		return 3;
	else if ((result = cuCtxCreate(&context, CU_CTX_SCHED_AUTO, device)) != CUDA_SUCCESS)
		return 4;
	else if ((result = cuCtxPopCurrent(&context)) != CUDA_SUCCESS)
		return 5;
	else if ((result = cuvidCtxLockCreate(&lock, context)) != CUDA_SUCCESS)
		return 6;
/*
	frameQueue = new CUVIDFrameQueue(lock);

	decoder.InitVideoDecoder(configuration.inputFileName, lock, frameQueue, configuration.width, configuration.height);

	int decodedW, decodedH, decodedFRN, decodedFRD, isProgressive;
	decoder.GetCodecParam(&decodedW, &decodedH, &decodedFRN, &decodedFRD, &isProgressive);
	if (decodedFRN <= 0 || decodedFRD <= 0) {
		decodedFRN = 30;
		decodedFRD = 1;
	}

	if (configuration.width <= 0 || configuration.height <= 0) {
		configuration.width = decodedW;
		configuration.height = decodedH;
	}

	fpsRatio = 1.f;
	if (configuration.fps <= 0) {
		configuration.fps = decodedFRN / decodedFRD;
	}
	else 
		fpsRatio = static_cast<float>(configuration.fps) * decodedFRD / decodedFRN;

	configuration.pictureStruct = (isProgressive ? NV_ENC_PIC_STRUCT_FRAME : 0);
	frameQueue->init(configuration.width, configuration.height);
*/
	fpsRatio = 1.f;
	encoder = new VideoEncoder(lock);
	assert(encoder->GetHWEncoder());

	if((nvStatus = encoder->GetHWEncoder()->Initialize(context, NV_ENC_DEVICE_TYPE_CUDA)) != NV_ENC_SUCCESS)
		return 6;

	configuration.presetGUID = encoder->GetHWEncoder()->GetPresetGUID(configuration.encoderPreset, configuration.codec);

	frameQueue = new CUVIDFrameQueue(lock);
	frameQueue->init(configuration.width, configuration.height);

	auto isProgressive = true;
	configuration.pictureStruct = (isProgressive ? NV_ENC_PIC_STRUCT_FRAME : 0);

	return 0;
}

void Transcoder::InitializeDecoder(const std::string& inputFilename)
{
	NVENCSTATUS nvStatus;

	this->inputFilename = inputFilename;
	configuration.inputFileName = const_cast<char*>(inputFilename.c_str());

	decoder.InitVideoDecoder(configuration.inputFileName, lock, frameQueue, configuration.width, configuration.height);

	/*
	int decodedW, decodedH, decodedFRN, decodedFRD, isProgressive;
	decoder.GetCodecParam(&decodedW, &decodedH, &decodedFRN, &decodedFRD, &isProgressive);
	if (decodedFRN <= 0 || decodedFRD <= 0) {
		decodedFRN = 30;
		decodedFRD = 1;
	}*/

	assert(configuration.width > 0);
	assert(configuration.height > 0);
	auto isProgressive = true;

	/*
	if (configuration.width <= 0 || configuration.height <= 0) {
		configuration.width = decodedW;
		configuration.height = decodedH;
	}

	fpsRatio = 1.f;
	if (configuration.fps <= 0) {
		configuration.fps = decodedFRN / decodedFRD;
	}
	else
		fpsRatio = static_cast<float>(configuration.fps) * decodedFRD / decodedFRN;
		*/

	//configuration.pictureStruct = (isProgressive ? NV_ENC_PIC_STRUCT_FRAME : 0);
	configuration.pictureStruct = (isProgressive ? NV_ENC_PIC_STRUCT_FRAME : 0);
}

int Transcoder::InitializeEncoder(const std::string& outputFilename)
{
	NVENCSTATUS nvStatus;

	this->outputFilename = outputFilename;

	configuration.outputFileName = const_cast<char*>(outputFilename.c_str());
	configuration.fOutput = fopen(configuration.outputFileName, "wb");
	encoder->GetHWEncoder()->m_fOutput = configuration.fOutput;

	if ((nvStatus = encoder->GetHWEncoder()->CreateEncoder(&configuration)) != NV_ENC_SUCCESS)
		return 7;
	else if ((nvStatus = encoder->AllocateIOBuffers(&configuration)) != NV_ENC_SUCCESS)
		return 8;
	else
		return 0;
}

int Transcoder::transcode(const std::string& inputFilename, const std::string& outputFilename)
{
	int result;
	NVENCSTATUS nvStatus;

	frameQueue->reset();

	if ((result = InitializeEncoder(outputFilename)) != 0)
		return result;

	InitializeDecoder(inputFilename);

	pthread_t pid;
    pthread_create(&pid, nullptr, DecodeProc, static_cast<void*>(&decoder));

    //start encoding thread
    auto frmProcessed = 0;
    auto frmActual = 0;
    while(!(frameQueue->isEndOfDecode() && frameQueue->isEmpty())) 
    	{
        CUVIDPARSERDISPINFO pInfo;
        if(frameQueue->dequeue(&pInfo)) {
            CUdeviceptr dMappedFrame = 0;
            unsigned int pitch;
            CUVIDPROCPARAMS oVPP = { 0 };
            oVPP.progressive_frame = pInfo.progressive_frame;
            oVPP.second_field = 0;
            oVPP.top_field_first = pInfo.top_field_first;
            oVPP.unpaired_field = (pInfo.progressive_frame == 1 || pInfo.repeat_first_field <= 1);

            cuvidMapVideoFrame(decoder.GetDecoder(), pInfo.picture_index, &dMappedFrame, &pitch, &oVPP);

            EncodeFrameConfig stEncodeConfig = { 0 };
            auto picType = (pInfo.progressive_frame || pInfo.repeat_first_field >= 2 ? NV_ENC_PIC_STRUCT_FRAME :
                (pInfo.top_field_first ? NV_ENC_PIC_STRUCT_FIELD_TOP_BOTTOM : NV_ENC_PIC_STRUCT_FIELD_BOTTOM_TOP));

            stEncodeConfig.dptr = dMappedFrame;
            stEncodeConfig.pitch = pitch;
            stEncodeConfig.width = configuration.width;
            stEncodeConfig.height = configuration.height;

            auto dropOrDuplicate = MatchFPS(fpsRatio, frmProcessed, frmActual);
			for (auto i = 0; i <= dropOrDuplicate; i++) {
				encoder->EncodeFrame(&stEncodeConfig, picType);
                frmActual++;
            }
            frmProcessed++;

            cuvidUnmapVideoFrame(decoder.GetDecoder(), dMappedFrame);
			frameQueue->releaseFrame(&pInfo);
       }
    }

	encoder->EncodeFrame(nullptr, NV_ENC_PIC_STRUCT_FRAME, true);

	pthread_join(pid, nullptr);

	if ((nvStatus = encoder->ReleaseIOBuffers()) != NV_ENC_SUCCESS)
		return -1;

	return 0;
}
