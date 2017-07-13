#include <pthread.h>
#include "dynlink_cuda.h"
#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "VideoEncoderSession.h"

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

int Transcoder::transcode(const std::string &inputFilename, EncodeWriter &writer) {
    frameQueue.reset();

    VideoEncoderSession session(encoder(), writer);

    decoder.InitVideoDecoder(inputFilename);

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
                session.Encode(stEncodeConfig, picType);
                frmActual++;
            }
            frmProcessed++;

            cuvidUnmapVideoFrame(decoder.GetDecoder(), dMappedFrame);
            frameQueue.releaseFrame(&pInfo);
        }
    }

    session.Flush();

    pthread_join(pid, nullptr);

    decoder.Deinitialize();

    return 0;
}
