#include <pthread.h>
#include <time.h>

#include "dynlink_cuda.h"
#include <stdio.h>

#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include <climits>

#include "Transcoder.h"

#include <ctime>

void *DecodeProc(void *arg) {
  auto *decoder = static_cast<CudaDecoder *>(arg);
  decoder->Start();

  return nullptr;
}

int MatchFPS(const float fpsRatio, int decodedFrames, int encodedFrames) {
  if (fpsRatio < 1.f) {
    // need to drop frame
    if (decodedFrames * fpsRatio < (encodedFrames + 1)) {
      return -1;
    }
  } else if (fpsRatio > 1.f) {
    // need to duplicate frame
    auto duplicate = 0;
    while (decodedFrames * fpsRatio > encodedFrames + duplicate + 1) {
      duplicate++;
    }

    return duplicate;
  }

  return 0;
}

///
///
///
///

Transcoder::Transcoder(GPUContext& context, EncodeConfig& configuration)
        : lock(context), encoder(lock.get()), frameQueue(lock.get()), configuration(configuration), fpsRatio(1) {
    outputFilename.reserve(1024);

    NVENCSTATUS nvStatus;

    assert(encoder.GetHWEncoder());

    if ((nvStatus = encoder.GetHWEncoder()->Initialize(context.get(), NV_ENC_DEVICE_TYPE_CUDA)) != NV_ENC_SUCCESS)
        ;

    configuration.presetGUID = encoder.GetHWEncoder()->GetPresetGUID(configuration.encoderPreset, configuration.codec);

    frameQueue.init(configuration.width, configuration.height);

    auto isProgressive = true;
    configuration.pictureStruct = (isProgressive ? NV_ENC_PIC_STRUCT_FRAME : 0);
}

void Transcoder::InitializeDecoder(const std::string &inputFilename) {
    NVENCSTATUS nvStatus;

    this->inputFilename = inputFilename;
    configuration.inputFileName = const_cast<char *>(inputFilename.c_str());

    decoder.InitVideoDecoder(configuration.inputFileName, lock.get(), &frameQueue, configuration.width, configuration.height);

    assert(configuration.width > 0);
    assert(configuration.height > 0);
    auto isProgressive = true;

    configuration.pictureStruct = (isProgressive ? NV_ENC_PIC_STRUCT_FRAME : 0);
}

int Transcoder::InitializeEncoder(const std::string &outputFilename) {
    NVENCSTATUS nvStatus;

    this->outputFilename = outputFilename;

    configuration.outputFileName = const_cast<char *>(outputFilename.c_str());
    configuration.fOutput = fopen(configuration.outputFileName, "wb");
    encoder.GetHWEncoder()->m_fOutput = configuration.fOutput;

    if ((nvStatus = encoder.GetHWEncoder()->CreateEncoder(&configuration)) != NV_ENC_SUCCESS)
        return 7;
    else if ((nvStatus = encoder.AllocateIOBuffers(&configuration)) != NV_ENC_SUCCESS)
        return 8;
    else
        return 0;
}

int Transcoder::transcode(const std::string &inputFilename, const std::string &outputFilename) {
    int result;
    NVENCSTATUS nvStatus;

    frameQueue.reset();

    // Destroy previously-initialized encoder, if any exists
    //encoder->GetHWEncoder()->NvEncDestroyEncoder();

    if ((result = InitializeEncoder(outputFilename)) != 0)
        return result;

    InitializeDecoder(inputFilename);

    pthread_t pid;
    pthread_create(&pid, nullptr, DecodeProc, static_cast<void *>(&decoder));

    // start encoding thread
    auto frmProcessed = 0;
    auto frmActual = 0;
    while (!(frameQueue.isEndOfDecode() && frameQueue.isEmpty())) {
        CUVIDPARSERDISPINFO pInfo;
        if (frameQueue.dequeue(&pInfo)) {
            CUdeviceptr dMappedFrame = 0;
            unsigned int pitch;
            CUVIDPROCPARAMS oVPP = {0};
            oVPP.progressive_frame = pInfo.progressive_frame;
            oVPP.second_field = 0;
            oVPP.top_field_first = pInfo.top_field_first;
            oVPP.unpaired_field = (pInfo.progressive_frame == 1 || pInfo.repeat_first_field <= 1);

            cuvidMapVideoFrame(decoder.GetDecoder(), pInfo.picture_index, &dMappedFrame, &pitch, &oVPP);

            EncodeFrameConfig stEncodeConfig = {0};
            auto picType =
                    (pInfo.progressive_frame || pInfo.repeat_first_field >= 2
                     ? NV_ENC_PIC_STRUCT_FRAME
                     : (pInfo.top_field_first ? NV_ENC_PIC_STRUCT_FIELD_TOP_BOTTOM : NV_ENC_PIC_STRUCT_FIELD_BOTTOM_TOP));

            stEncodeConfig.dptr = dMappedFrame;
            stEncodeConfig.pitch = pitch;
            stEncodeConfig.width = configuration.width;
            stEncodeConfig.height = configuration.height;

            auto dropOrDuplicate = MatchFPS(fpsRatio, frmProcessed, frmActual);
            for (auto i = 0; i <= dropOrDuplicate; i++) {
                encoder.EncodeFrame(&stEncodeConfig, picType);
                frmActual++;
            }
            frmProcessed++;

            cuvidUnmapVideoFrame(decoder.GetDecoder(), dMappedFrame);
            frameQueue.releaseFrame(&pInfo);
        }
    }

    encoder.EncodeFrame(nullptr, NV_ENC_PIC_STRUCT_FRAME, true);

    pthread_join(pid, nullptr);

    if ((nvStatus = encoder.ReleaseIOBuffers()) != NV_ENC_SUCCESS)
        return -1;

    return 0;
}
