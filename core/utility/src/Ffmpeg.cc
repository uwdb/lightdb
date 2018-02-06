#include "Ffmpeg.h"
extern "C" {
#include <libavformat/avformat.h>
}

//TODO this all shoudl be in the video subdirectory, it's all video stuff...
namespace lightdb::utility {
    namespace ffmpeg {
        //TemporalRange duration(std::istream& stream) {
        //    return TemporalRange{0, 20}; //TODO broken
        //}


        void initialize() {
            //TODO clean this up
            static bool initialized = false;
            if (!initialized) {
                initialized = true;
                av_register_all(); //TODO register only necessary formats instead of everything
            }
        }

        /*
        class Video {
            FrameIterator begin() {

            }

            FrameIterator end() {

            }
        };*/

        FrameIterator::PacketIterator::PacketIterator(std::istream &input, size_t buffer_size)
        //TODO does it make sense to init the parser and context over and over?
                : codec_(*CHECK_NOTNULL(
                avcodec_find_decoder(AV_CODEC_ID_H264))), //AV_CODEC_ID_MPEG4))), //AV_CODEC_ID_MPEG2VIDEO))),
                  parser_(*CHECK_NOTNULL(av_parser_init(codec_.id))),
                  context_(CHECK_NOTNULL(avcodec_alloc_context3(&codec_))),
                  packet_(CHECK_NOTNULL(av_packet_alloc())),
                  input_(input),
                  buffer_(buffer_size + AV_INPUT_BUFFER_PADDING_SIZE, 0),
                  iterator_(buffer_.end()),
                  end_(buffer_.end()) {
            CHECK_GE(avcodec_open2(context_, &codec_, nullptr), 0) << "Could not open codec";
            LOG(WARNING) << "Packet iterator not advanced by default; fix this before relying on it";
            //++*this;
        }

        FrameIterator::PacketIterator::~PacketIterator() {
            av_parser_close(&parser_);
            avcodec_free_context(&context_);
            av_packet_free(&packet_);
        }

        //       AVCodecContext& FrameIterator::PacketIterator::context() const { return *context_; }

        FrameIterator::PacketIterator::operator bool() const {
            return iterator_ != end_ || input_.eofbit;
        }

        AVPacket &FrameIterator::PacketIterator::operator*() const {
            return *packet_;
        }

        FrameIterator::PacketIterator &FrameIterator::PacketIterator::operator++() {
            int bytes_parsed;

            if (iterator_ == end_) {
                input_.read(reinterpret_cast<char *>(buffer_.data()), buffer_.size());
                iterator_ = buffer_.begin();
                end_ = iterator_ + input_.gcount();
            }

            CHECK_GE(bytes_parsed = av_parser_parse2(
                    &parser_, context_, &packet_->data, &packet_->size,
                    iterator_.base(), end_ - iterator_,
                    AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0), 0) << "Error parsing stream";

            //printf("parsed %d\n", bytes_parsed);
            iterator_ += bytes_parsed;
            return *this;
        }

        FrameIterator::PacketIterator &FrameIterator::PacketIterator::operator++(int) {
            //FrameIterator::PacketIterator clone = *this;
            //++*this;
            return ++*this;
        }


        FrameIterator::FrameIterator(std::istream &input, size_t buffer_size)
                : packets_{input, buffer_size},
                  frame_(CHECK_NOTNULL(av_frame_alloc())),
                  result_(AVERROR(EAGAIN)) {
            LOG(WARNING) << "Ffmpeg iterator not advanced by default; fix this before relying on it";
            //++*this;
        }

        /*
            FrameIterator::FrameIterator(std::istream &input, size_t buffer_size)
                : FrameIterator(FrameIterator::PacketIterator{input, buffer_size})
            { }

            FrameIterator(FrameIterator::PacketIterator &packets)
                : packets_(packets),
                  frame_(CHECK_NOTNULL(av_frame_alloc())),
                  result_(AVERROR(EAGAIN))
            { ++*this; }

            FrameIterator::FrameIterator(FrameIterator::PacketIterator &&packets)
                : packets_(packets),
                  frame_(CHECK_NOTNULL(av_frame_alloc())),
                  result_(AVERROR(EAGAIN))
            { ++*this; }
        */

        FrameIterator::~FrameIterator() {
            av_frame_free(&frame_);
        }

        FrameIterator::operator bool() const {
            return packets_ || result_ != AVERROR_EOF;
        }

        AVFrame &FrameIterator::operator*() const {
            return *frame_;
        }

        FrameIterator &FrameIterator::operator++() {
            char e[256];

            result_ = avcodec_receive_frame(&packets_.context(), frame_);
            if (result_ == AVERROR(EAGAIN)) {
                auto &p = *packets_;
                auto foo = avcodec_send_packet(&packets_.context(), &p);
                packets_++;
                //auto foo = avcodec_send_packet(&packets_.context(), &*packets_++);
                av_strerror(foo, e, 256);
                //printf("sendpacket %d %s\n", foo, e);
                return ++*this;
            } else {
                av_strerror(result_, e, 256);
                //printf("frame %s\n", e);
                //CHECK_GE(result_ = avcodec_receive_frame(&packets_.context(), frame_), 0);
                return *this;
            }
        }

        std::unique_ptr<FrameIterator> decode(std::istream &input, size_t buffer_size) {
            initialize(); //TODO do this statically rather than over and over
            return std::make_unique<FrameIterator>(input, buffer_size);
        }

        static void decodeframe(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt) {
            int ret;

            ret = avcodec_send_packet(dec_ctx, pkt);
            if (ret < 0) {
                fprintf(stderr, "Error sending a packet for decoding\n");
                exit(1);
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    return;
                else if (ret < 0) {
                    fprintf(stderr, "Error during decoding\n");
                    exit(1);
                }

                //printf("saving frame %3d\n", dec_ctx->frame_number);
                //fflush(stdout);

                /* the picture is allocated by the decoder. no need to
                   free it */
                //snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
                //pgm_save(frame->data[0], frame->linesize[0],
                //         frame->width, frame->height, buf);
            }
        }

        void decodexxx(std::istream &input, size_t buffer_size = 1024 * 1024) {
            //const char *filename, *outfilename;
            const AVCodec *codec;
            AVCodecParserContext *parser;
            AVCodecContext *c = nullptr;
            AVFrame *frame;
            std::vector<uint8_t> buffer(buffer_size + AV_INPUT_BUFFER_PADDING_SIZE, 0);
            //uint8_t buffer[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
            //uint8_t *data;
            //size_t data_size;
            AVPacket *pkt;
            size_t parse_count;

            CHECK_NOTNULL(pkt = av_packet_alloc());

            /* find the MPEG-1 video decoder */
            CHECK_NOTNULL(codec = avcodec_find_decoder(AV_CODEC_ID_MPEG2VIDEO));
            CHECK_NOTNULL(parser = av_parser_init(codec->id));
            CHECK_NOTNULL(c = avcodec_alloc_context3(codec));

            /* For some codecs, such as msmpeg4 and mpeg4, width and height
               MUST be initialized there because this information is not
               available in the bitstream. */

            CHECK_GE(avcodec_open2(c, codec, NULL), 0) << "Could not open codec";
            CHECK_NOTNULL(frame = av_frame_alloc());

            //while (!feof(f)) {
            while (!input.eof()) {
                input.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

                for (auto iterator = buffer.begin(), end = buffer.begin() + input.gcount();
                     iterator != end; iterator += parse_count) {
                    CHECK_GE(parse_count = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                                                            iterator.base(), end - iterator, AV_NOPTS_VALUE,
                                                            AV_NOPTS_VALUE, 0), 0) << "Error parsing stream";
                    //data      += ret;
                    //data_size -= ret;

                    if (pkt->size)
                        decodeframe(c, frame, pkt);
                }


                /* use the parser to split the data into frames */
                /*data = buffer;
                while (data_size > 0) {
                    CHECK_GE(av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                                           data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0), 0) << "Error parsing stream";
                    data      += ret;
                    data_size -= ret;

                    if (pkt->size)
                        decodeframe(c, frame, pkt, outfilename);
                }*/
            }

            /* flush the decoder */
            decodeframe(c, frame, nullptr);

            av_parser_close(parser);
            avcodec_free_context(&c);
            av_frame_free(&frame);
            av_packet_free(&pkt);
        }
    }; // namespace ffmpeg

    StreamMetadata::StreamMetadata(const std::string &filename, const size_t index, const bool probe) {
        int result;
        char buffer[AV_ERROR_MAX_STRING_SIZE];

        ffmpeg::initialize(); //TODO do this statically rather than over and over

        AVFormatContext* context = avformat_alloc_context();

        if((result = avformat_open_input(&context, filename.c_str(), nullptr, nullptr)) < 0) {
            LOG(ERROR) << av_make_error_string (buffer, AV_ERROR_MAX_STRING_SIZE, result);
            //TODO leaks context
            throw std::runtime_error(buffer); //TODO
        } else if((probe && (result = avformat_find_stream_info(context, nullptr)) < 0)) {
            LOG(ERROR) << av_make_error_string (buffer, AV_ERROR_MAX_STRING_SIZE, result);
            //TODO leaks context
            throw std::runtime_error(buffer); //TODO
        }

        if(context->nb_streams < index + 1)
            throw std::runtime_error("Index is larger than number of streams"); //TODO
        else if(context->streams[0]->codecpar->height <= 0 ||
                context->streams[0]->codecpar->width <= 0)
            throw std::runtime_error("Frame size not detected"); //TODO
        else if(context->streams[0]->nb_frames < 0)
            throw std::runtime_error("No frames detected"); //TODO
        else if(context->bit_rate < 0)
            throw std::runtime_error("No bitrate detected"); //TODO
        else if(context->streams[index]->codecpar->codec_id != AV_CODEC_ID_H264 &&
                context->streams[index]->codecpar->codec_id != AV_CODEC_ID_HEVC)
            throw std::runtime_error("Hardcoded support only for H264 and HEVC"); //TODO

        codec =  context->streams[index]->codecpar->codec_id == AV_CODEC_ID_H264 ? "h264" : "hevc";
        height = static_cast<unsigned int>(context->streams[index]->codecpar->height);
        width = static_cast<unsigned int>(context->streams[index]->codecpar->width);
        framerate = rational(context->streams[index]->r_frame_rate.num, context->streams[index]->r_frame_rate.den);
        frames = static_cast<size_t>(context->streams[index]->nb_frames);
        duration = rational(context->duration != AV_NOPTS_VALUE ? context->duration : 0, AV_TIME_BASE);
        bitrate = static_cast<size_t>(context->bit_rate);
        this->index = index;

        avformat_close_input(&context);
        avformat_free_context(context);
    }
} // namespace lightdb::utility