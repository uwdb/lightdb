#ifndef LIGHTDB_FFMPEG_H
#define LIGHTDB_FFMPEG_H

#include "Geometry.h"
#include "number.h"
extern "C" {
#include <libavcodec/avcodec.h>
}
#include "Configuration.h"
#include "Codec.h"
#include <glog/logging.h>
#include <vector>
#include <istream>

namespace lightdb::utility {
    //TODO drop this struct and just return a Configuration instance
    struct StreamMetadata {
        std::string codec_;
        unsigned int height, width;
        rational framerate;
        size_t frames;
        rational duration;
        size_t bitrate;
        size_t index;

        StreamMetadata(const std::string &filename, size_t index, bool probe=false);

        Configuration configuration() const {
            return Configuration{width, height, 0, 0, bitrate,
                                 {static_cast<const unsigned int>(framerate.numerator()),
                                  static_cast<const unsigned int>(framerate.denominator())}};
        }

        Codec codec() const {
            if(codec_ == "h264")
                return Codec::h264();
            else if(codec_ == "hevc")
                return Codec::hevc();
            else
                throw NotImplementedError("StreamMetadata does not support codec conversion");
        }
    };

    namespace ffmpeg {
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

        //explicit FrameIterator(FrameIterator::PacketIterator &packets);
        //explicit FrameIterator(PacketIterator &&packets);

        void initialize();

        std::unique_ptr<FrameIterator> decode(std::istream &stream, size_t buffer_size = 1024 * 1024);
        //std::optional<MediaMetadata> get_metadata(std::string &filename, bool probe_stream=false);
        //TemporalRange duration(std::istream& stream); //TODO hardcoded
    }; // namespace ffmpeg
}; // namespace lightdb::utility

#endif //LIGHTDB_FFMPEG_H
