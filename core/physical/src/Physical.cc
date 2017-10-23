#include "Physical.h"
#include "TileVideoEncoder.h"
#include <glog/logging.h>

namespace visualcloud {
    namespace physical {
        template<typename ColorSpace>
        void EquirectangularTiledLightField<ColorSpace>::hardcode_hack(const unsigned int framerate, const unsigned int gop, const unsigned int height, const unsigned int width, const unsigned int rows, const unsigned int columns, const unsigned int max_bitrate, const std::string &intermediate_format, const std::string &output_format) {
            EquirectangularTiledLightField<ColorSpace>::framerate = framerate;
            EquirectangularTiledLightField<ColorSpace>::gop = gop;
            EquirectangularTiledLightField<ColorSpace>::height = height;
            EquirectangularTiledLightField<ColorSpace>::width = width;

            auto decodeCodec = intermediate_format == "h264" ? cudaVideoCodec_H264 : cudaVideoCodec_HEVC;
            auto encodeCodec = output_format == "h264" ? NV_ENC_H264 : NV_ENC_HEVC;

            EquirectangularTiledLightField<ColorSpace>::encodeConfiguration = new EncodeConfiguration(height, width, encodeCodec, framerate, gop, max_bitrate*1024*1024);
            EquirectangularTiledLightField<ColorSpace>::decodeConfiguration = new DecodeConfiguration(*encodeConfiguration, decodeCodec);

            EquirectangularTiledLightField<ColorSpace>::context = new GPUContext(0);
            EquirectangularTiledLightField<ColorSpace>::tiler = new TileVideoEncoder(*context, *decodeConfiguration, *encodeConfiguration, rows, columns);
        }

        template<typename ColorSpace>
        EncodedLightField EquirectangularTiledLightField<ColorSpace>::apply(const std::string &format) {
            //GPUContext context(0);
            LOG(INFO) << "Executing tiling physical operator with " << rows_ << " rows and " << columns_ << " columns";

            if(rows_ != tiler->rows() || columns_ != tiler->columns())
                LOG(ERROR) << "No tiler available for this configuration.";

            //assert(rows_ == tiler->rows());
            //assert(columns_ == tiler->columns());

            //LOG(INFO) << "Ignoring framerate numerator; this will break for other videos.";
            //LOG(INFO) << "Hardcoded framerate and GOP; not even paying attention to input video (fake) framerate";
            //auto framerate = 24; //video_.frames().framerate().denominator();
            //auto gop = framerate;
            //auto resolution = video_.frames().resolution();
            //auto height = resolution.second, width = resolution.first;
            //auto height = 1920, width = 3840;
            //auto bitrate = 500 * 1024 * 1024u;

            //TODO fix
            //auto foo = std::string(".h264");
            //auto bar = std::equal(foo.rbegin(), foo.rend(), video_.filename().rbegin());
            //auto decodeCodec = bar
             //       ? cudaVideoCodec_H264 : cudaVideoCodec_HEVC;
            //auto encodeCodec = format == "h264" ? NV_ENC_H264 : NV_ENC_HEVC;

            //EncodeConfiguration encodeConfiguration(height, width, encodeCodec, framerate, gop, bitrate);
            //DecodeConfiguration decodeConfiguration(encodeConfiguration, decodeCodec);

            //TileVideoEncoder tiler(context, decodeConfiguration, encodeConfiguration, rows_, columns_);
            //TODO StreamDecodeReader reader(video_.frames().stream());
            FileDecodeReader reader(video_.filename());
            std::vector<std::shared_ptr<EncodeWriter>> writers;

            for(auto i = 0; i < rows_ * columns_; i++)
                writers.emplace_back(std::make_shared<MemoryEncodeWriter>(tiler->api()));
                //writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler->api(), std::string("out") + std::to_string(i)));

            tiler->tile(reader, writers); //, NV_ENC_SUCCESS);

            LOG(INFO) << "Completed tiling physical operator";

            std::vector<std::unique_ptr<std::istream>> decodes{};
            for(auto i = 0; i < rows_ * columns_; i++)
                decodes.emplace_back(std::make_unique<std::ifstream>(std::string("out") + std::to_string(i)));
                //decodes.emplace_back(Decode<EquirectangularGeometry>(std::string("out") + std::to_string(i) + "." + format).apply());

            return CompositeEncodedLightField::create(std::move(decodes));
        }

        template<typename ColorSpace>
        EncodedLightField StitchedLightField<ColorSpace>::apply() {
            return apply(0);
        }

        template<typename ColorSpace>
        EncodedLightField StitchedLightField<ColorSpace>::apply(const visualcloud::rational &interval) {
            auto height = 0u, width = 0u;

            //TODO broken; this doesn't take tile coordinates into account
            /*for(auto &video: videos_) {
                auto resolution = video_.frames().resolution();
                height += resolution.second;
                width += resolution.first;
            }*/

            LOG(INFO) << "Hardcoded video resolution; this will break for other videos.";
            LOG(INFO) << "Hardcoded tile resolution; this will break for other tilings.";
            LOG(INFO) << "Wonky GOP/time interval splitting";
            auto tile_width = 4, tile_height = 1;
            height = 960, width = 1920;
            auto dinterval = (double)interval.numerator() / interval.denominator();

            std::string command{"/home/bhaynes/projects/visualcloud/stitch.sh " + std::to_string(height) + " " + std::to_string(width) + " " + std::to_string(tile_height) + " " + std::to_string(tile_width) + " " + std::to_string(dinterval)};
            system(command.c_str());

            return SingletonEncodedLightField::create("stitched.hevc");
        }

        template class EquirectangularTiledLightField<YUVColorSpace>;
        template class StitchedLightField<YUVColorSpace>;
    } // namespace physical
} // namespace visualcloud

