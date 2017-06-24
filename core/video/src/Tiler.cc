#include <pthread.h>

#include <iostream>
#include <sstream>
#include <string.h>

#include "TileVideoEncoder.h"
#include "VideoDecoder.h"
#include <algorithm>

typedef struct Statistics { unsigned long long start, end, frequency; } Statistics;

std::vector<std::string> split(const std::string &input, char delimiter) {
  std::vector<std::string> elements;
  std::stringstream stream(input);
  std::string value;

  while (std::getline(stream, value, delimiter))
    elements.push_back(value);

  return elements;
}
int error(const char *message, const int exitCode) {
  std::cerr << message;
  return exitCode;
}

void *DecodeWorker(void *arg) {
  auto *decoder = (CudaDecoder *)arg;
  decoder->Start();

  return NULL;
}

int TilerMatchFPS(const float fpsRatio, const int decodedFrames, const int encodedFrames) {
  if (fpsRatio < 1.f) {
    // need to drop frame
    if (decodedFrames * fpsRatio < (encodedFrames + 1))
      return -1;
  } else if (fpsRatio > 1.f) {
    // need to duplicate frame
    auto duplicate = 0;
    while (decodedFrames * fpsRatio > encodedFrames + duplicate + 1)
      duplicate++;

    return duplicate;
  }

  return 0;
}

int PrintHelp() {
  std::cout << "Usage : NvTranscoder \n"
               "-i <string>                  Specify input .h264 file\n"
               "-o <string>                  Specify output bitstream file\n"
               "\n### Optional parameters ###\n"
               "-size <int int>              Specify output resolution <width "
               "height>\n"
               "-codec <integer>             Specify the codec \n"
               "                                 0: H264\n"
               "                                 1: HEVC\n"
               "-preset <string>             Specify the preset for encoder "
               "settings\n"
               "                                 hq : nvenc HQ \n"
               "                                 hp : nvenc HP \n"
               "                                 lowLatencyHP : nvenc low latency HP "
               "\n"
               "                                 lowLatencyHQ : nvenc low latency HQ "
               "\n"
               "                                 lossless : nvenc Lossless HP \n"
               "-fps <integer>               Specify encoding frame rate\n"
               "-goplength <integer>         Specify gop length\n"
               "-numB <integer>              Specify number of B frames\n"
               "-bitrate <integer>           Specify the encoding average bitrate\n"
               "-vbvMaxBitrate <integer>     Specify the vbv max bitrate\n"
               "-vbvSize <integer>           Specify the encoding vbv/hrd buffer "
               "size\n"
               "-rcmode <integer>            Specify the rate control mode\n"
               "                                 0:  Constant QP\n"
               "                                 1:  Single pass VBR\n"
               "                                 2:  Single pass CBR\n"
               "                                 4:  Single pass VBR minQP\n"
               "                                 8:  Two pass frame quality\n"
               "                                 16: Two pass frame size cap\n"
               "                                 32: Two pass VBR\n"
               "-qp <integer>                Specify qp for Constant QP mode\n"
               "-i_qfactor <float>           Specify qscale difference between "
               "I-frames and P-frames\n"
               "-b_qfactor <float>           Specify qscale difference between "
               "P-frames and B-frames\n"
               "-i_qoffset <float>           Specify qscale offset between I-frames "
               "and P-frames\n"
               "-b_qoffset <float>           Specify qscale offset between P-frames "
               "and B-frames\n"
               "-deviceID <integer>          Specify the GPU device on which "
               "encoding will take place\n"
               "-help                        Prints Help Information\n\n";
  return 1;
}

int DisplayConfiguration(const EncodeConfig &configuration, const TileDimensions &dimensions) {
  printf("Encoding input           : \"%s\"\n", configuration.inputFileName);
  printf("         output          : \"%s\"\n", configuration.outputFileName);
  printf("         codec           : \"%s\"\n", configuration.codec == NV_ENC_HEVC ? "HEVC" : "H264");
  printf("         size            : %dx%d\n", configuration.width, configuration.height);
  printf("         bitrate         : %d bits/sec\n", configuration.bitrate);
  printf("         vbvMaxBitrate   : %d bits/sec\n", configuration.vbvMaxBitrate);
  printf("         vbvSize         : %d bits\n", configuration.vbvSize);
  printf("         fps             : %d frames/sec\n", configuration.fps);
  printf("         rcMode          : %s\n",
         configuration.rcMode == NV_ENC_PARAMS_RC_CONSTQP
             ? "CONSTQP"
             : configuration.rcMode == NV_ENC_PARAMS_RC_VBR
                   ? "VBR"
                   : configuration.rcMode == NV_ENC_PARAMS_RC_CBR
                         ? "CBR"
                         : configuration.rcMode == NV_ENC_PARAMS_RC_VBR_MINQP
                               ? "VBR MINQP"
                               : configuration.rcMode == NV_ENC_PARAMS_RC_2_PASS_QUALITY
                                     ? "TWO_PASS_QUALITY"
                                     : configuration.rcMode == NV_ENC_PARAMS_RC_2_PASS_FRAMESIZE_CAP
                                           ? "TWO_PASS_FRAMESIZE_CAP"
                                           : configuration.rcMode == NV_ENC_PARAMS_RC_2_PASS_VBR ? "TWO_PASS_VBR"
                                                                                                 : "UNKNOWN");
  if (configuration.gopLength == NVENC_INFINITE_GOPLENGTH)
    printf("         goplength       : INFINITE GOP \n");
  else
    printf("         goplength       : %d \n", configuration.gopLength);
  printf("         B frames        : %d \n", configuration.numB);
  printf("         QP              : %d \n", configuration.qp);
  printf("         preset          : %s\n",
         (configuration.presetGUID == NV_ENC_PRESET_LOW_LATENCY_HQ_GUID)
             ? "LOW_LATENCY_HQ"
             : (configuration.presetGUID == NV_ENC_PRESET_LOW_LATENCY_HP_GUID)
                   ? "LOW_LATENCY_HP"
                   : (configuration.presetGUID == NV_ENC_PRESET_HQ_GUID)
                         ? "HQ_PRESET"
                         : (configuration.presetGUID == NV_ENC_PRESET_HP_GUID)
                               ? "HP_PRESET"
                               : (configuration.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID) ? "LOSSLESS_HP"
                                                                                              : "LOW_LATENCY_DEFAULT");
  printf("         Tiles           : %lu, %lu\n", dimensions.rows, dimensions.columns);
  printf("\n");

  return 0;
}

float InitializeDecoder(CudaDecoder &decoder, CUVIDFrameQueue &queue, CUvideoctxlock &lock,
                        EncodeConfig &configuration) {
  int decodedW, decodedH, decodedFRN, decodedFRD, isProgressive;

  decoder.InitVideoDecoder(configuration.inputFileName, lock, &queue, configuration.width, configuration.height);

  decoder.GetCodecParam(&decodedW, &decodedH, &decodedFRN, &decodedFRD, &isProgressive);
  if (decodedFRN <= 0 || decodedFRD <= 0) {
    decodedFRN = 30;
    decodedFRD = 1;
  }

  if (configuration.width <= 0 || configuration.height <= 0) {
    configuration.width = decodedW;
    configuration.height = decodedH;
  }

  float fpsRatio = 1.f;
  if (configuration.fps <= 0)
    configuration.fps = decodedFRN / decodedFRD;
  else
    fpsRatio = (float)configuration.fps * decodedFRD / decodedFRN;

  configuration.pictureStruct = isProgressive ? NV_ENC_PIC_STRUCT_FRAME : 0;
  queue.init(configuration.width, configuration.height);

  return fpsRatio;
}

struct EncodeContext {
  CudaDecoder &decoder;
  std::vector<TileVideoEncoder *> &encoders;
  CUVIDFrameQueue &frameQueue;
  EncodeConfig &configuration;
  float fpsRatio;
  Statistics &statistics;
};

void EncodeWorker(EncodeContext &context)
// CudaDecoder& decoder, TileVideoEncoder& encoder, CUVIDFrameQueue& queue,
// EncodeConfig& configuration,
//            float fpsRatio)
{
  auto frmProcessed = 0;
  auto frmActual = 0;

  while (!(context.frameQueue.isEndOfDecode() && context.frameQueue.isEmpty())) {
    CUVIDPARSERDISPINFO frame;

    if (context.frameQueue.dequeue(&frame)) {
      CUdeviceptr mappedFrame = 0;
      CUVIDPROCPARAMS oVPP = {0};
      unsigned int pitch;

      oVPP.progressive_frame = frame.progressive_frame;
      oVPP.second_field = 0;
      oVPP.top_field_first = frame.top_field_first;
      oVPP.unpaired_field = (frame.progressive_frame == 1 || frame.repeat_first_field <= 1);

      cuvidMapVideoFrame(context.decoder.GetDecoder(), frame.picture_index, &mappedFrame, &pitch, &oVPP);

      EncodeFrameConfig stEncodeConfig = {0};
      auto pictureType =
          (frame.progressive_frame || frame.repeat_first_field >= 2
               ? NV_ENC_PIC_STRUCT_FRAME
               : (frame.top_field_first ? NV_ENC_PIC_STRUCT_FIELD_TOP_BOTTOM : NV_ENC_PIC_STRUCT_FIELD_BOTTOM_TOP));

      stEncodeConfig.dptr = mappedFrame;
      stEncodeConfig.pitch = pitch;
      stEncodeConfig.width = context.configuration.width;
      stEncodeConfig.height = context.configuration.height;

      auto dropOrDuplicate = TilerMatchFPS(context.fpsRatio, frmProcessed, frmActual);
      if (dropOrDuplicate != 1)
        printf("DoD %d\n", dropOrDuplicate);
      for (auto i = 0; i <= dropOrDuplicate; i++) {
        for (auto *encoder : context.encoders)
          encoder->EncodeFrame(&stEncodeConfig, pictureType);

        // context.encoder1.EncodeFrame(&stEncodeConfig, pictureType);
        // context.encoder2.EncodeFrame(&stEncodeConfig, pictureType);
        // context.encoder3.EncodeFrame(&stEncodeConfig, pictureType);
        frmActual++;
      }
      frmProcessed++;

      cuvidUnmapVideoFrame(context.decoder.GetDecoder(), mappedFrame);
      context.frameQueue.releaseFrame(&frame);
    }
  }

  for (auto *encoder : context.encoders)
    encoder->EncodeFrame(nullptr, NV_ENC_PIC_STRUCT_FRAME, true);
  // context.encoder1.EncodeFrame(nullptr, NV_ENC_PIC_STRUCT_FRAME, true);
  // context.encoder2.EncodeFrame(nullptr, NV_ENC_PIC_STRUCT_FRAME, true);
  // context.encoder3.EncodeFrame(nullptr, NV_ENC_PIC_STRUCT_FRAME, true);
}

int ExecuteWorkers(CudaDecoder &decoder, std::vector<TileVideoEncoder *> &encoders, CUVIDFrameQueue &frameQueue,
                   EncodeConfig &configuration, float fpsRatio, Statistics &statistics) {
  pthread_t decode_pid;

  NvQueryPerformanceCounter(&statistics.start);

  // Start decoding thread
  pthread_create(&decode_pid, nullptr, DecodeWorker, static_cast<void *>(&decoder));

  // Execute encoder in main thread
  EncodeContext context = {decoder, encoders, frameQueue, configuration, fpsRatio, statistics};
  EncodeWorker(context);

  pthread_join(decode_pid, nullptr);

  return 0;
}

int DisplayStatistics(CudaDecoder &decoder, TileVideoEncoder &encoder, Statistics &statistics) {
  if (encoder.GetEncodedFrames() > 0) {
    NvQueryPerformanceCounter(&statistics.end);
    NvQueryPerformanceFrequency(&statistics.frequency);

    auto elapsedTime = static_cast<double>(statistics.end - statistics.start) / statistics.frequency;
    printf("Total time: %fms, Decoded Frames: %d, Encoded Frames: %ld, Average "
           "FPS: %f\n",
           elapsedTime * 1000, decoder.m_decodedFrames, encoder.GetEncodedFrames(),
           static_cast<float>(encoder.GetEncodedFrames()) / elapsedTime);
  }

  return 0;
}

int ParseTileParameters(EncodeConfig &configuration, TileDimensions &tileDimensions) {
  auto values = split(configuration.outputFileName, ',');

  if (values.size() != 3)
    return error("Expected three arguments in output filename (e.g., '4,8,%d.h265')\n", -1);
  else {
    tileDimensions.rows = stoi(values.at(0));
    tileDimensions.columns = stoi(values.at(1));
    tileDimensions.count = tileDimensions.rows * tileDimensions.columns;
    strcpy(configuration.outputFileName, values.at(2).c_str());
  }

  return 0;
}

int InitializeEncoder(CUcontext &cudaContext, EncodeConfig &configuration, unsigned long bitrate, int encoderIndex,
                      TileVideoEncoder &encoder, const TileDimensions &tileDimensions) {
  NVENCSTATUS status;

  //    configuration.presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
  //    //encoder->GetPresetGUID();

  // Initialize encoder
  if ((status = encoder.Initialize(cudaContext, NV_ENC_DEVICE_TYPE_CUDA)) != NV_ENC_SUCCESS)
    return error("encoder.Initialize", -1);
  else if ((status = encoder.CreateEncoders(configuration)) != NV_ENC_SUCCESS)
    return error("CreateEncoders", -1);
  else if ((status = encoder.AllocateIOBuffers(&configuration)) != NV_ENC_SUCCESS)
    return error("encoder.AllocateIOBuffers", -1);
  else
    return 0;
}

int ExecuteTiler(std::vector<EncodeConfig> &configurations, const TileDimensions tileDimensions) {
  typedef void *CUDADRIVER;
  CUDADRIVER hHandleDriver = nullptr;
  CUcontext cudaCtx;
  CUdevice device;
  CUcontext curCtx;
  CUvideoctxlock lock;
  CUresult result;
  NVENCSTATUS status;
  CudaDecoder decoder;
  CUVIDFrameQueue frameQueue(lock);
  Statistics statistics;
  auto fpsRatio = 1.f;
  std::vector<TileVideoEncoder *> encoders;

  // Initialize CUDA
  if ((result = cuInit(0, __CUDA_API_VERSION, hHandleDriver)) != CUDA_SUCCESS)
    return error("Error in cuInit", result);
  else if ((result = cuvidInit(0)) != CUDA_SUCCESS)
    return error("Error in cuInit", result);
  else if ((result = cuDeviceGet(&device, configurations.at(0).deviceID)) != CUDA_SUCCESS)
    return error("cuDeviceGet", result);
  else if ((result = cuCtxCreate(&cudaCtx, CU_CTX_SCHED_AUTO, device)) != CUDA_SUCCESS)
    return error("cuCtxCreate", result);
  else if ((result = cuCtxPopCurrent(&curCtx)) != CUDA_SUCCESS)
    return error("cuCtxPopCurrent", result);
  else if ((result = cuvidCtxLockCreate(&lock, curCtx)) != CUDA_SUCCESS)
    return error("cuvidCtxLockCreate", result);
  else if ((fpsRatio = InitializeDecoder(decoder, frameQueue, lock, configurations.at(0))) < 0)
    return error("InitializeDecoder", -1);
  // else if (DisplayConfiguration(configurations.at(0), tileDimensions) != 0)
  //	return error("DisplayConfiguration", -1);

  auto index = 0;
  for (auto &configuration : configurations) {
    // TODO leaks
    auto *encoder = new TileVideoEncoder(lock, tileDimensions.columns, tileDimensions.rows);
    if (InitializeEncoder(cudaCtx, configuration, configuration.bitrate, index++, *encoder, tileDimensions))
      return error("InitializeEncoder", -1);
    else
      encoders.push_back(encoder);
  }

  if (ExecuteWorkers(decoder, encoders, frameQueue, configurations.at(0), fpsRatio, statistics) != 0)
    return error("ExecuteWorkers", -1);

  for (auto &encoder : encoders) {
    if ((status = encoder->Deinitialize()) != NV_ENC_SUCCESS)
      return error("encoder.Deinitialize", result);
    delete encoder;
  }

  if ((result = cuvidCtxLockDestroy(lock)) != CUDA_SUCCESS)
    return error("cuvidCtxLockDestroy", result);
  else if ((result = cuCtxDestroy(cudaCtx)) != CUDA_SUCCESS)
    return error("cuCtxDestroy", result);
  else
    return 0;
}

/*
//TODO moved to EncodeConfig::Create
EncodeConfig MakeTilerConfiguration(char *inputFilename, char *outputFilenameFormat, const unsigned int height,
                                    const unsigned int width, const size_t tileRows, const size_t tileColumns,
                                    const unsigned int codec, char *preset, const unsigned int fps,
                                    const unsigned int gop_length, const size_t bitrate, const unsigned int rcmode,
                                    const unsigned int deviceId) {
  EncodeConfig configuration = {0};

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
  configuration.encoderPreset = preset;
  configuration.deviceID = deviceId;

  configuration.inputFileName = inputFilename;
  configuration.outputFileName = outputFilenameFormat;

  return configuration;
}
*/

int ExecuteTiler(const std::string inputFilename, const std::string outputFilenameFormat, unsigned int height,
                 unsigned int width, size_t tileRows, size_t tileColumns, unsigned int codec, std::string preset,
                 unsigned int fps, unsigned int gop_length, size_t bitrate, unsigned int rcmode,
                 unsigned int deviceId) {
  EncodeConfig configuration(
          const_cast<char *>(inputFilename.c_str()), const_cast<char *>(outputFilenameFormat.c_str()), height, width,
          tileRows, tileColumns, codec, const_cast<char *>(preset.c_str()), fps, gop_length, bitrate, rcmode, deviceId);
  //EncodeConfig configuration = MakeTilerConfiguration(
  //    const_cast<char *>(inputFilename.c_str()), const_cast<char *>(outputFilenameFormat.c_str()), height, width,
  //    tileRows, tileColumns, codec, const_cast<char *>(preset.c_str()), fps, gop_length, bitrate, rcmode, deviceId);

  TileDimensions tileDimensions = {tileRows, tileColumns};

  std::vector<EncodeConfig> configurations;
  configurations.push_back(configuration);

  return ExecuteTiler(configurations, tileDimensions);
}

int main(int argc, char *argv[]) {
  printf("main\n");
  return 0;
}

/*
int foo(int argc, char *argv[]) {
  NVENCSTATUS status;
  CUresult result;
  TileDimensions tileDimensions;

  EncodeConfig configuration = {0};
  configuration.endFrameIdx = INT_MAX;
  configuration.bitrate = 5000000;
  configuration.rcMode = NV_ENC_PARAMS_RC_CONSTQP;
  configuration.gopLength = NVENC_INFINITE_GOPLENGTH;
  configuration.codec = NV_ENC_H264;
  configuration.fps = 0;
  configuration.qp = 28;
  configuration.i_quant_factor = DEFAULT_I_QFACTOR;
  configuration.b_quant_factor = DEFAULT_B_QFACTOR;
  configuration.i_quant_offset = DEFAULT_I_QOFFSET;
  configuration.b_quant_offset = DEFAULT_B_QOFFSET;
  configuration.presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
  configuration.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;

  std::vector<EncodeConfig> configurations;
  configurations.push_back(configuration);

  // Verify arguments
  if ((status = EncodeAPI::ParseArguments(&configuration, argc, argv)) != NV_ENC_SUCCESS)
    return PrintHelp();
  else if (!configuration.inputFileName || !configuration.outputFileName)
    return PrintHelp();
  else if (ParseTileParameters(configuration, tileDimensions) != 0)
    return error("ParseTileParameters", -1);
  else
    return ExecuteTiler(configurations, tileDimensions);
}
*/