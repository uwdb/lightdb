extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <fstream>
#include <gtest/gtest.h>

class FfmpegTestFixture: public testing::Test {
public:
    FfmpegTestFixture()
    { }

/*    SwsContext* swsContext;

    void initFrame(AVFrame **rgbFrame, AVFrame *frame)
    {
        if(*rgbFrame == nullptr) {
            swsContext = sws_getContext(frame->width,
                                             frame->height,
                                             (enum AVPixelFormat)frame->format,
                                             frame->width,
                                             frame->height,
                                             AV_PIX_FMT_BGR24,
                                             SWS_BICUBIC,
                                             nullptr, nullptr, nullptr);

            *rgbFrame = av_frame_alloc();
            (*rgbFrame)->width = frame->width;
            (*rgbFrame)->height = frame->height;
            (*rgbFrame)->format = AV_PIX_FMT_BGR24;
            (*rgbFrame)->channels = 3;
            (*rgbFrame)->pict_type = frame->pict_type;
            //(char*)(*rgbFrame)->data = new char[frame->width * frame->height * 3];
            //memset((*rgbFrame)->data[0], 0, frame->width * frame->height * 3);
            //(*rgbFrame)->linesize[0] = (*rgbFrame)->width * 3;

            auto* rgbBuffer = (uint8_t *)av_malloc( avpicture_get_size(AV_PIX_FMT_RGB24, frame->width, frame->height));
            avpicture_fill((AVPicture*)rgbFrame, rgbBuffer, AV_PIX_FMT_RGB24, frame->width, frame->height);
        }
    }*/
};

TEST_F(FfmpegTestFixture, testTiling) {
    /*
    //TODO Move this to a separate project, this is all a hack

    av_register_all();

    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    AVCodecParserContext *parser = av_parser_init(codec->id);
    AVCodecContext *context = avcodec_alloc_context3(codec);
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    AVFrame *rgbFrame = nullptr;

    avcodec_open2(context, codec, nullptr);

    std::ifstream f("/home/bhaynes/projects/visualcloud/benchmarks/datasets/timelapse/timelapse4K.h264");
    std::string data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    uint8_t *current = (uint8_t*)data.data(), *end = current + data.size();
    char error[256];
    auto frames = 0u;

    while(current < end)
    {
        auto bytes_parsed = av_parser_parse2(
                parser, context, &packet->data, &packet->size,
                current, end - current,
                AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        current += bytes_parsed;

        for(;;)
        {
            auto result = avcodec_receive_frame(context, frame);
            av_strerror(result, error, 256);
            //printf("%s\n", error);
            if (result == AVERROR(EAGAIN)) {
                avcodec_send_packet(context, packet);
                break;
            } else {
                printf("%d %lu\n", frames++, end - current);
                initFrame(&rgbFrame, frame);

                / *auto scale = sws_scale(swsContext,
                                   frame->data,
                                   frame->linesize,
                                   0,
                                   frame->height,
                                   rgbFrame->data,
                                   rgbFrame->linesize);* /
            }
        }
    }


    av_frame_free(&rgbFrame);
    av_frame_free(&frame);
    av_parser_close(parser);
    avcodec_free_context(&context);
    av_packet_free(&packet);
*/
    GTEST_SKIP();
}
