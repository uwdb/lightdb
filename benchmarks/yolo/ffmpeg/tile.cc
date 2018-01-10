extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include "darknet.h"
}

#include <fstream>

SwsContext* swsContext;

void frame_to_image(AVFrame *frame, size_t h, size_t w, size_t c, size_t step, image &im)
{
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(j = 0; j < w; ++j){
            for(k= 0; k < c; ++k){
                im.data[k*w*h + i*w + j] = frame->data[0][i*frame->linesize[0]+j*c+k]/255.;
            }
        }
    }
}

void image_to_frame(image &im, size_t h, size_t w, size_t c, size_t step, AVFrame *frame)
{
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(j = 0; j < w; ++j){
            for(k= 0; k < c; ++k){
                frame->data[0][i*frame->linesize[0]+j*c+k] = im.data[k*w*h + i*w + j] * 255;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    auto *network = load_network("/home/bhaynes/projects/darknet/cfg/tiny-yolo.cfg", "/home/bhaynes/projects/darknet/tiny-yolo.weights", 0);
    auto metadata = get_metadata("/home/bhaynes/projects/darknet/cfg/coco.data");
    auto *boxes = make_boxes(network);
    auto **probs = make_probs(network);
    auto num = num_boxes(network);
    auto **alphabet = load_alphabet();
    auto image = make_image(416, 416, 3);
    auto bigimage = make_image(3840, 2048, 3);
    auto threshold = 0.5f, hier_thresh=0.5f, nms=.45f;

    av_register_all();

    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_H264);

    AVCodecParserContext *parser = av_parser_init(codec->id);
    AVCodecContext *context = avcodec_alloc_context3(codec);
    AVCodecContext *encodeContext = avcodec_alloc_context3(encoder);
    AVFormatContext *formatContext = avformat_alloc_context();
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    avcodec_open2(context, codec, nullptr);
    avcodec_open2(encodeContext, encoder, nullptr);

    AVStream *stream = avformat_new_stream(formatContext, encoder);
    stream->id = 0;
    avcodec_get_context_defaults3(stream->codec, encoder);
    stream->codec->width = 3840;
    stream->codec->height = 2048;
    stream->time_base.num = 1;
    stream->time_base.den = 30;
    stream->codec->gop_size = 30;
    stream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
    stream->pts = {1, 30};
    stream->cur_dts = 0;
    stream->first_dts = 0;

    formatContext->oformat = av_guess_format(NULL, "out.h264", NULL);
    formatContext->video_codec = encoder;
    strcpy(formatContext->filename, "out.h264");
    avio_open(&formatContext->pb, formatContext->filename, AVIO_FLAG_WRITE);

    auto r = avformat_write_header(formatContext, nullptr);

    std::ifstream f(argv[1]);
    std::string data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    uint8_t *current = (uint8_t*)data.data(), *end = current + data.size();

    auto swsContext = sws_getContext(3840,
                                2048,
                                AV_PIX_FMT_YUV420P,
                                416,
                                416,
                                AV_PIX_FMT_RGB24,
                                SWS_BICUBIC,
                                NULL, NULL, NULL);

    auto *rgbFrame = av_frame_alloc();
    rgbFrame->width = 416;
    rgbFrame->height = 416;
    rgbFrame->format = AV_PIX_FMT_RGB24;
    rgbFrame->channels = 3;
    rgbFrame->linesize[0] = 416;

    uint8_t* rgbBuffer = (uint8_t *)av_malloc( avpicture_get_size(AV_PIX_FMT_RGB24, rgbFrame->width, rgbFrame->height));
    avpicture_fill((AVPicture*)rgbFrame, rgbBuffer, AV_PIX_FMT_RGB24, rgbFrame->width, rgbFrame->height);

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

            if (result == AVERROR(EAGAIN)) {
                avcodec_send_packet(context, packet);
                break;
            } else {
                rgbFrame->pict_type = frame->pict_type;

                auto scale = sws_scale(swsContext,
                                   frame->data,
                                   frame->linesize,
                                   0,
                                   frame->height,
                                   rgbFrame->data,
                                   rgbFrame->linesize);

                frame_to_image(rgbFrame, 416, 416, 3, 3*416, image);

                network_detect(network, image, threshold, hier_thresh, nms, boxes, probs);
                draw_detections(image, num, threshold, boxes, probs, nullptr, metadata.names,
                        alphabet, metadata.classes);

                AVPacket pkt;
                av_init_packet(&pkt);

                pkt.flags |= AV_PKT_FLAG_KEY;
                pkt.stream_index= 0; //st->index;
                pkt.data= (uint8_t *)frame;
                pkt.size= sizeof(AVPicture);
                av_interleaved_write_frame(formatContext, &pkt);
            }
        }
    }

    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);

    free_network(network);
    av_frame_free(&rgbFrame);
    av_frame_free(&frame);
    av_parser_close(parser);
    avcodec_free_context(&context);
    avcodec_free_context(&encodeContext);
    av_packet_free(&packet);
}