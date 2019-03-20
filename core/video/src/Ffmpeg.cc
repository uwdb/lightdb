#include "Ffmpeg.h"
extern "C" {
#include <libavformat/avformat.h>
}

namespace lightdb::video::ffmpeg {
    DecodeConfiguration GetConfigurationFromContext(const AVStream*, unsigned int bitrate);

    DecodeConfiguration GetStreamConfiguration(const std::string &filename, const size_t index, const bool probe) {
        auto configurations = GetStreamConfigurations(filename, probe);

        if (configurations.size() < index)
            throw InvalidArgumentError("Index is larger than number of streams", "index");
        else
            return configurations.at(index);
    }

    std::vector<DecodeConfiguration> GetStreamConfigurations(const std::string &filename, const bool probe) {
        int result;
        char error[AV_ERROR_MAX_STRING_SIZE];
        std::vector<DecodeConfiguration> configurations;

        auto context = avformat_alloc_context();

        try {
            if ((result = avformat_open_input(&context, filename.c_str(), nullptr, nullptr)) < 0) {
                throw FfmpegRuntimeError(av_make_error_string(error, AV_ERROR_MAX_STRING_SIZE, result));
            } else if ((probe && (result = avformat_find_stream_info(context, nullptr)) < 0)) {
                throw FfmpegRuntimeError(av_make_error_string(error, AV_ERROR_MAX_STRING_SIZE, result));
            } else if (context->bit_rate < 0) {
                throw FfmpegRuntimeError("No bitrate detected");
            }

            for(auto i = 0u; i < context->nb_streams; i++)
                configurations.emplace_back(GetConfigurationFromContext(context->streams[i],
                                            static_cast<unsigned int>(context->bit_rate)));

            avformat_close_input(&context);
            avformat_free_context(context);

            return configurations;
        } catch(const std::exception&) {
            avformat_close_input(&context);
            avformat_free_context(context);
            throw;
        }
    }

    DecodeConfiguration GetConfigurationFromContext(const AVStream *stream, const unsigned int bitrate) {
        if (stream->codecpar->height <= 0 ||
            stream->codecpar->width <= 0)
            throw FfmpegRuntimeError("Frame size not detected");
        else if (stream->nb_frames < 0)
            throw FfmpegRuntimeError("No frames detected");

        auto codec = Codec::get(stream->codecpar->codec_id);
        if(!codec.has_value())
            throw FfmpegRuntimeError("Unsupported codec type");
        else
            return DecodeConfiguration{
                    static_cast<unsigned int>(stream->codecpar->height),
                    static_cast<unsigned int>(stream->codecpar->width),
                    0u,
                    0u,
                    lightdb::rational{stream->r_frame_rate.num,
                                      stream->r_frame_rate.den},
                    bitrate,
                    codec.value()
            };
    }

    /*
//    class Video {
//        FrameIterator begin() {
//
//        }
//
//        FrameIterator end() {
//
//        }
//    };

    class FrameIterator : std::iterator<std::output_iterator_tag, AVFrame> {
    public:
        FrameIterator(std::istream &input, size_t buffer_size = 1024 * 1024);

        ~FrameIterator();

        operator bool() const;

        AVFrame &operator*() const;

        FrameIterator &operator++();

        FrameIterator &operator++(int);

        std::istream &stream() const { return packets_.stream(); }

        double duration() const {
            LOG(INFO) << "Using hardcoded duration; this will break for other videos.";
            return 20;
        } //TODO hardcoded duration
        rational framerate() const {
            LOG(INFO) << "Using hardcoded framerate; this will break for other videos.";
            return rational(1, 30);
        } //TODO hardcoded duration
        std::pair<size_t, size_t> resolution() const {
            LOG(INFO) << "Using hardcoded resolution; this will break for other videos.";
            return {1920, 1080};
        } //TODO hardcoded duration

    private:
        class PacketIterator : std::iterator<std::output_iterator_tag, AVPacket> {
        public:
            PacketIterator(std::istream &input, size_t buffer_size = 1024 * 1024);

            ~PacketIterator();

            std::istream &stream() const { return input_; }

            AVCodecContext &context() const { return *context_; }

            operator bool() const;

            AVPacket &operator*() const;

            PacketIterator &operator++();

            PacketIterator &operator++(int);

        private:
            const AVCodec &codec_;
            AVCodecParserContext &parser_;
            AVCodecContext *context_;
            AVPacket *packet_;
            std::istream &input_;
            std::vector<uint8_t> buffer_;
            std::vector<uint8_t>::const_iterator iterator_, end_;
        } packets_;

        AVFrame *frame_;
        int result_;
    };

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

//        FrameIterator::FrameIterator(std::istream &input, size_t buffer_size)
//            : FrameIterator(FrameIterator::PacketIterator{input, buffer_size})
//        { }
//
//        FrameIterator(FrameIterator::PacketIterator &packets)
//            : packets_(packets),
//              frame_(CHECK_NOTNULL(av_frame_alloc())),
//              result_(AVERROR(EAGAIN))
//        { ++*this; }
//
//        FrameIterator::FrameIterator(FrameIterator::PacketIterator &&packets)
//            : packets_(packets),
//              frame_(CHECK_NOTNULL(av_frame_alloc())),
//              result_(AVERROR(EAGAIN))
//        { ++*this; }

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
//        initialize(); //TODO do this statically rather than over and over
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

            // the picture is allocated by the decoder. no need to
              // free it
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

        // find the MPEG-1 video decoder
        CHECK_NOTNULL(codec = avcodec_find_decoder(AV_CODEC_ID_MPEG2VIDEO));
        CHECK_NOTNULL(parser = av_parser_init(codec->id));
        CHECK_NOTNULL(c = avcodec_alloc_context3(codec));

        // For some codecs, such as msmpeg4 and mpeg4, width and height
        //   MUST be initialized there because this information is not
        //   available in the bitstream.

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


            // use the parser to split the data into frames
            //data = buffer;
            //while (data_size > 0) {
            //    CHECK_GE(av_parser_parse2(parser, c, &pkt->data, &pkt->size,
             //                          data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0), 0) << "Error parsing stream";
             //   data      += ret;
             //   data_size -= ret;

             //   if (pkt->size)
             //       decodeframe(c, frame, pkt, outfilename);
            //}
        }

        // flush the decoder
        decodeframe(c, frame, nullptr);

        av_parser_close(parser);
        avcodec_free_context(&c);
        av_frame_free(&frame);
        av_packet_free(&pkt);
    }
*/
} // namespace lightdb::video::ffmpeg