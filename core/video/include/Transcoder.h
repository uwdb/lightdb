#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <string>

#include "dynlink_cuda.h"
#include "VideoDecoder.h"
#include "VideoEncoder.h"

class Transcoder
{
public:
	Transcoder(unsigned int height, unsigned int width,
			   unsigned int codec, std::string preset, unsigned int fps, unsigned int gop_length,
			   unsigned long bitrate, unsigned int rcmode, unsigned int deviceId);
	~Transcoder()
	{
		if(encoder != nullptr)
			encoder->Deinitialize();

		delete encoder;
		delete frameQueue;

		cuvidCtxLockDestroy(lock);
		cuCtxDestroy(context);
	}

	int initialize();
	int transcode(const std::string& inputFilename, const std::string& outputFilename);

private:
	int InitializeEncoder(const std::string& outputFilename);
	void InitializeDecoder(const std::string& inputFilename);

	std::string inputFilename;
	std::string outputFilename;
	const std::string preset;

	CUcontext context;
	CUvideoctxlock lock;
	VideoEncoder* encoder; //TODO
	CudaDecoder decoder;
	FrameQueue* frameQueue; //TODO 
	EncodeConfig configuration;
	float fpsRatio;
};

#endif 
