#ifndef VISUALCLOUD_VIDEODECODERSESSION_H
#define VISUALCLOUD_VIDEODECODERSESSION_H

#include <thread>
#include "VideoDecoder.h"
#include "DecodeReader.h"
#include "dynlink_cuda.h"

class VideoDecoderSession {
public:
    VideoDecoderSession(CudaDecoder& decoder, DecodeReader& reader)
            : decoder_(decoder), reader(reader), parser(CreateParser(this, decoder_)),
              decodedFrameCount_(0),
              worker{&VideoDecoderSession::DecodeAll, this}
    { }

    ~VideoDecoderSession() {
        decoder_.frame_queue().endDecode();
        worker.join();
        cuvidDestroyVideoParser(parser);
    }

    DecodedFrame decode() {
        return DecodedFrame(decoder_, decoder_.frame_queue().dequeue_wait<CUVIDPARSERDISPINFO>());
    }

    const CudaDecoder &decoder() const { return decoder_; }

protected:
    CudaDecoder &decoder_;
    DecodeReader &reader;
    CUvideoparser parser;
    size_t decodedFrameCount_;
    std::thread worker;

    void DecodeAll() {
        CUresult status;
        std::optional <DecodeReaderPacket> packet;

        do {
            packet = reader.ReadPacket();
            if (packet.has_value()) {
                if ((status = cuvidParseVideoData(parser, &*packet)) != CUDA_SUCCESS) {
                    LOG(ERROR) << "cuvidParseVideoData failed with code " << status;
                    //throw std::runtime_error(std::to_string(status) + "DecodeAll"); //TODO
                }
            }
        } while (!decoder_.frame_queue().isEndOfDecode() && packet.has_value());

        decoder_.frame_queue().endDecode();
    }

private:
    static CUvideoparser CreateParser(VideoDecoderSession *session, CudaDecoder &decoder) {
        CUresult status;
        CUvideoparser parser = nullptr;
        CUVIDPARSERPARAMS parameters = {
            .CodecType = decoder.configuration().codec,
            .ulMaxNumDecodeSurfaces = decoder.configuration().decode_surfaces,
            .ulClockRate = 0,
            .ulErrorThreshold = 0,
            .ulMaxDisplayDelay = 1,
            .uReserved1 = {},
            .pUserData = session,
            .pfnSequenceCallback = HandleVideoSequence,
            .pfnDecodePicture = HandlePictureDecode,
            .pfnDisplayPicture = HandlePictureDisplay,
            0
        };

        decoder.frame_queue().reset();

        if ((status = cuvidCreateVideoParser(&parser, &parameters)) != CUDA_SUCCESS) {
            LOG(INFO) << "cuvidCreateVideoParser";
            throw std::runtime_error(std::to_string(status) + "CreateParser"); // TODO
        }

        return parser;
    }

    static int CUDAAPI HandleVideoSequence(void *userData, CUVIDEOFORMAT *format) {
        auto* session = static_cast<VideoDecoderSession*>(userData);

        assert(format->display_area.bottom - format->display_area.top >= 0);
        assert(format->display_area.right - format->display_area.left >= 0);

        if(session == nullptr)
            LOG(ERROR) << "Unexpected null session data during video decode (HandleVideoSequence)";
        //assert(session);

        else if ((format->codec != session->decoder_.configuration().codec) ||
            ((static_cast<unsigned int>(format->display_area.right - format->display_area.left)) != session->decoder_.configuration().width) ||
            ((static_cast<unsigned int>(format->display_area.bottom - format->display_area.top)) != session->decoder_.configuration().height) ||
            (format->coded_width < session->decoder_.configuration().width) ||
            (format->coded_height < session->decoder_.configuration().height) ||
            (format->chroma_format != session->decoder_.configuration().chroma_format)) {
            LOG(ERROR) << "Video format changed but not currently supported";
            throw std::runtime_error("Video format changed but not currently supported"); //TODO
        }

        return 1;
    }

    static int CUDAAPI HandlePictureDecode(void *userData, CUVIDPICPARAMS *parameters) {
        //LOG(INFO) << "HandlePictureDecode";
        CUresult status;
        auto* session = static_cast<VideoDecoderSession*>(userData);

        if(session == nullptr)
            LOG(ERROR) << "Unexpected null session data during video decode (HandlePictureDecode)";
        //assert(session);
        else {
            session->decoder_.frame_queue().waitUntilFrameAvailable(parameters->CurrPicIdx);
            if((status = cuvidDecodePicture(session->decoder_.handle(), parameters)) != CUDA_SUCCESS)
                LOG(ERROR) << "cuvidDecodePicture failed (" << status << ")";
            //assert(cuvidDecodePicture(session->decoder.handle(), parameters) == CUDA_SUCCESS);
        }

        return 1;
    }

    static int CUDAAPI HandlePictureDisplay(void *userData, CUVIDPARSERDISPINFO *frame) {
        //LOG(INFO) << "HandlePictureDisplay";
        auto* session = static_cast<VideoDecoderSession*>(userData);

        if(session == nullptr)
            LOG(ERROR) << "Unexpected null session data during video decode (HandlePictureDisplay)";
        //assert(session);
        else {
            session->decoder_.frame_queue().enqueue(frame);
            session->decodedFrameCount_++;
        }

        return 1;
    }
};

#endif //VISUALCLOUD_VIDEODECODERSESSION_H
