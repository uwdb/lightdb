#ifndef VISUALCLOUD_FFMPEG_H
#define VISUALCLOUD_FFMPEG_H

#include "Geometry.h"
#include "rational.h"
extern "C" {
#include <libavcodec/avcodec.h>
}
#include <vector>
#include <istream>

namespace visualcloud::utility::ffmpeg {
    class FrameIterator : std::iterator<std::output_iterator_tag, AVFrame> {
    public:
        FrameIterator(std::istream &input, size_t buffer_size=1024*1024);
        ~FrameIterator();

        operator bool() const;
        AVFrame &operator*() const;
        FrameIterator& operator++();
        FrameIterator& operator++(int);

        double duration() const { return 20; } //TODO hardcoded duration
        rational framerate() const { return rational(1, 30); } //TODO hardcoded duration

    private:
        class PacketIterator : std::iterator<std::output_iterator_tag, AVPacket> {
        public:
            PacketIterator(std::istream &input, size_t buffer_size=1024*1024);
            ~PacketIterator();

            AVCodecContext& context() const { return *context_; }

            operator bool() const;
            AVPacket& operator*() const;
            PacketIterator& operator++();
            PacketIterator& operator++(int);

        private:
            const AVCodec &codec_;
            AVCodecParserContext &parser_;
            AVCodecContext *context_;
            AVPacket *packet_;
            std::istream& input_;
            std::vector<uint8_t> buffer_;
            std::vector<uint8_t>::const_iterator iterator_, end_;
        } packets_;

        AVFrame *frame_;
        int result_;
    };

    //explicit FrameIterator(FrameIterator::PacketIterator &packets);
    //explicit FrameIterator(PacketIterator &&packets);

    void initialize();
    std::unique_ptr<FrameIterator> decode(std::istream& stream, size_t buffer_size=1024*1024);
    //TemporalRange duration(std::istream& stream); //TODO hardcoded
};

#endif //VISUALCLOUD_FFMPEG_H
