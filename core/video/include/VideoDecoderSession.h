#ifndef VISUALCLOUD_VIDEODECODERSESSION_H
#define VISUALCLOUD_VIDEODECODERSESSION_H

#include <thread>
#include "VideoDecoder.h"
#include "DecodeReader.h"
#include "dynlink_cuda.h"

class VideoDecoderSession {
public:
    VideoDecoderSession(CudaDecoder& decoder, DecodeReader& reader)
            : decoder(decoder), reader(reader), parser(CreateParser(this, decoder)),
              worker{&VideoDecoderSession::DecodeAll, this} {
    }

    ~VideoDecoderSession() {
        worker.join();
        cuvidDestroyVideoParser(parser);
    }

    const DecodedFrame decode() {
        return DecodedFrame(decoder, decoder.frameQueue.dequeue_wait<CUVIDPARSERDISPINFO>());
    }

protected:
    CudaDecoder &decoder;
    DecodeReader &reader;
    CUvideoparser parser;
    size_t decodedFrameCount_;
    std::thread worker;

    void DecodeAll() {
        CUresult status;

        std::optional <CUVIDSOURCEDATAPACKET> packet;
        do {
            packet = reader.DecodeFrame();
            if (packet.has_value())
                if ((status = cuvidParseVideoData(parser, &*packet)) != CUDA_SUCCESS)
                    throw status; //TODO
        } while (packet.has_value());

        decoder.frameQueue.endDecode();
    }

private:
    static CUvideoparser CreateParser(VideoDecoderSession *session, CudaDecoder &decoder) {
        CUresult status;
        CUvideoparser parser = nullptr;
        CUVIDPARSERPARAMS parameters = {
            .CodecType = decoder.m_oVideoDecodeCreateInfo.CodecType,
            .ulMaxNumDecodeSurfaces = static_cast<unsigned int>(decoder.m_oVideoDecodeCreateInfo.ulNumDecodeSurfaces),
            .ulClockRate = 0,
            .ulErrorThreshold = 0,
            .ulMaxDisplayDelay = 1,
            .uReserved1 = {},
            .pUserData = session,
            .pfnSequenceCallback = HandleVideoSequence,
            .pfnDecodePicture = HandlePictureDecode,
            .pfnDisplayPicture = HandlePictureDisplay
        };

        decoder.frameQueue.reset();

        if ((status = cuvidCreateVideoParser(&parser, &parameters)) != CUDA_SUCCESS)
            throw status; // TODO

        return parser;
    }

    static int CUDAAPI HandleVideoSequence(void *userData, CUVIDEOFORMAT *format) {
        auto* session = static_cast<VideoDecoderSession*>(userData);

        assert(session);

        if ((format->codec != session->decoder.m_oVideoDecodeCreateInfo.CodecType) || // codec-type
            (format->coded_width != session->decoder.m_oVideoDecodeCreateInfo.ulWidth) ||
            (format->coded_height != session->decoder.m_oVideoDecodeCreateInfo.ulHeight) ||
            (format->chroma_format != session->decoder.m_oVideoDecodeCreateInfo.ChromaFormat))
                throw "Video format changed but not currently supported"; //TODO

        return 1;
    }

    static int CUDAAPI HandlePictureDecode(void *userData, CUVIDPICPARAMS *parameters) {
        auto* session = static_cast<VideoDecoderSession*>(userData);

        assert(session);

        session->decoder.frameQueue.waitUntilFrameAvailable(parameters->CurrPicIdx);
        assert(cuvidDecodePicture(session->decoder.handle(), parameters) == CUDA_SUCCESS);

        return 1;
    }

    static int CUDAAPI HandlePictureDisplay(void *userData, CUVIDPARSERDISPINFO *frame) {
        auto* session = static_cast<VideoDecoderSession*>(userData);

        assert(session);

        session->decoder.frameQueue.enqueue(frame);
        session->decodedFrameCount_++;

        return 1;
    }
};


#endif //VISUALCLOUD_VIDEODECODERSESSION_H
