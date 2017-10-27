#include "Physical.h"
#include "TileVideoEncoder.h"
#include "Operators.h"
#include <Configuration.h>
#include <glog/logging.h>
#include <optional>
#include <unistd.h>

namespace visualcloud {
    namespace physical {
        template<typename ColorSpace>
        void EquirectangularTiledLightField<ColorSpace>::hardcode_hack(const unsigned int framerate, const unsigned int gop, const unsigned int height, const unsigned int width, const unsigned int rows, const unsigned int columns, const unsigned int max_bitrate, const std::string &decode_format, const std::string &encode_format) {
            EquirectangularTiledLightField<ColorSpace>::framerate = framerate;
            EquirectangularTiledLightField<ColorSpace>::gop = gop;
            EquirectangularTiledLightField<ColorSpace>::height = height;
            EquirectangularTiledLightField<ColorSpace>::width = width;
            EquirectangularTiledLightField<ColorSpace>::rows = rows;
            EquirectangularTiledLightField<ColorSpace>::columns = columns;
            EquirectangularTiledLightField<ColorSpace>::decode_format = decode_format;
            EquirectangularTiledLightField<ColorSpace>::encode_format = encode_format;
            EquirectangularTiledLightField<ColorSpace>::max_bitrate = max_bitrate;

            //EquirectangularTiledLightField<ColorSpace>::decodeCodec = decode_format == "h264" ? cudaVideoCodec_H264 : cudaVideoCodec_HEVC;
            //EquirectangularTiledLightField<ColorSpace>::encodeCodec = encode_format == "h264" ? NV_ENC_H264 : NV_ENC_HEVC;

            //EquirectangularTiledLightField<ColorSpace>::encodeConfiguration = std::make_shared<EncodeConfiguration>(height, width, encodeCodec, framerate, gop, max_bitrate*1024*1024);
            //EquirectangularTiledLightField<ColorSpace>::decodeConfiguration = std::make_shared<DecodeConfiguration>(*encodeConfiguration, decodeCodec);

            //EquirectangularTiledLightField<ColorSpace>::context = std::make_shared<GPUContext>(0);
            //context->AttachToThread();
            //EquirectangularTiledLightField<ColorSpace>::tiler = std::make_shared<TileVideoEncoder>(*context, *decodeConfiguration, *encodeConfiguration, rows, columns);
        }

        template<typename ColorSpace>
        EncodedLightField EquirectangularTiledLightField<ColorSpace>::apply(const std::string &format) {
            //GPUContext context(0);
            LOG(INFO) << "Executing tiling physical operator with " << rows_ << " rows and " << columns_ << " columns";

            auto start = std::chrono::steady_clock::now();

            const auto rows = 4, columns = 4;
            const auto height = 2160, width = 3840;
            EquirectangularTiledLightField<ColorSpace>::columns = columns;
            EquirectangularTiledLightField<ColorSpace>::rows = rows;
            //const auto height = 2160, width = 3840;
            GPUContext context(0);
            //context.AttachToThread();
            std::vector<EncodeConfiguration> encodeConfigurations;
            for(auto r = 0; r < rows; r++) {
                for(auto c = 0; c < columns; c++) {
                    encodeConfigurations.emplace_back(EncodeConfiguration{height/rows, width/columns, NV_ENC_HEVC, 30, 30, 1024*1024});
                    encodeConfigurations.at(encodeConfigurations.size() - 1).bitrate = r == 0 && c == 1 ? 5000u*1024 : 50;
                    encodeConfigurations.at(encodeConfigurations.size() - 1).videoBufferingVerifier.maxBitrate = r == 0 && c == 1 ? 5000u*1024 : 50;
                    //encodeConfigurations.at(encodeConfigurations.size() - 1).videoBufferingVerifier.size = 500000;
                    encodeConfigurations.at(encodeConfigurations.size() - 1).quantization.rateControlMode = NV_ENC_PARAMS_RC_CBR2;
                    //encodeConfigurations.at(encodeConfigurations.size() - 1).quantization.rateControlMode = NV_ENC_PARAMS_RC_CBR;
                    //encodeConfigurations.at(encodeConfigurations.size() - 1).quantization.quantizationParameter = 50;
                    //encodeConfigurations.at(encodeConfigurations.size() - 1).bitrate = r == 1 && c == 1 ? 5000u : 50;
                    //encodeConfigurations.at(encodeConfigurations.size() - 1).bitrate = r == 1 && c == 1 ? 5000*1024u : 50000u;
                }
            }

            //EncodeConfiguration encodeConfiguration(1920, 3840, NV_ENC_HEVC, 30, 30, 1024*1024);
            DecodeConfiguration decodeConfiguration(encodeConfigurations.at(0), cudaVideoCodec_H264);
            decodeConfiguration.width = width;
            decodeConfiguration.height = height;

            TileVideoEncoder tiler(context, decodeConfiguration, encodeConfigurations, rows, columns);
                //[](auto row, auto column) -> std::optional<size_t> { return row == 1 && column == 1 ? 5000*1024u : 50000u; });

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
            LOG(INFO) << "Tile operator initialization took " << elapsed.count() << "ms";


            //FileDecodeReader reader("resources/test-pattern.h264");
            FileDecodeReader reader("/home/bhaynes/projects/visualcloud/test/resources/test-pattern-4K.h264");
            //FileDecodeReader reader("/home/bhaynes/projects/visualcloud/test/resources/test-pattern-4K.h264");
            std::vector<std::shared_ptr<EncodeWriter>> writers;

            for(auto i = 0; i < rows * columns; i++)
                //writers.emplace_back(std::make_shared<MemoryEncodeWriter>(tiler.api()));
                writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler.api(), std::string("out") + std::to_string(i)));

            tiler.tile(reader, writers);

            std::vector<bytestring> decodes{};
            //std::vector<std::unique_ptr<std::istream>> decodes{};
            for(auto i = 0; i < rows_ * columns_; i++)
                decodes.emplace_back(bytestring{});
                //decodes.emplace_back(dynamic_cast<MemoryEncodeWriter*>(writers.at(i).get())->buffer());
                //decodes.emplace_back(std::make_unique<std::ifstream>(std::string("out") + std::to_string(i)));

            return CompositeEncodedLightField::create(std::move(decodes));



/*


            //if(rows_ != tiler->rows() || columns_ != tiler->columns())
            //    LOG(ERROR) << "No tiler available for this configuration.";

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

            auto start = std::chrono::steady_clock::now();

            auto decodeCodec = decode_format == "h264" ? cudaVideoCodec_H264 : cudaVideoCodec_HEVC;
            auto encodeCodec = encode_format == "h264" ? NV_ENC_H264 : NV_ENC_HEVC;
            EncodeConfiguration encodeConfiguration(height, width, encodeCodec, framerate, gop, max_bitrate*1024*1024);
            DecodeConfiguration decodeConfiguration(encodeConfiguration, decodeCodec);

            GPUContext context = GPUContext(0);
            context.AttachToThread();
            TileVideoEncoder tiler(context, decodeConfiguration, encodeConfiguration, rows, columns);

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
            LOG(INFO) << "Tile operator initialization took " << elapsed.count() << "ms";


            //EncodeConfiguration encodeConfiguration(height, width, encodeCodec, framerate, gop, bitrate);
            //DecodeConfiguration decodeConfiguration(encodeConfiguration, decodeCodec);

            //TileVideoEncoder tiler(context, decodeConfiguration, encodeConfiguration, rows_, columns_);
            //TODO StreamDecodeReader reader(video_.frames().stream());
            FileDecodeReader reader(video_.filename());
            std::vector<std::shared_ptr<EncodeWriter>> writers;

            //for(auto i = 0; i < rows_ * columns_; i++)
            for(auto i = 0; i < rows * columns; i++)
                writers.emplace_back(std::make_shared<MemoryEncodeWriter>(tiler.api()));
                //writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler->api(), std::string("out") + std::to_string(i)));

            tiler.tile(reader, writers); //, NV_ENC_SUCCESS);
            executed = true;

            LOG(INFO) << "Completed tiling physical operator";

            / * (std::vector<std::unique_ptr<std::istream>> decodes{};
            for(auto i = 0; i < rows_ * columns_; i++)
                decodes.emplace_back(std::make_unique<std::ifstream>(std::string("out") + std::to_string(i)));
                //decodes.emplace_back(Decode<EquirectangularGeometry>(std::string("out") + std::to_string(i) + "." + format).apply()); * /

            //TODO this winds up making a copy; fix
            std::vector<std::vector<char>> decodes{};
            for(auto i = 0; i < rows_ * columns_; i++)
                decodes.emplace_back(dynamic_cast<MemoryEncodeWriter*>(writers.at(i).get())->buffer());
            //decodes.emplace_back(Decode<EquirectangularGeometry>(std::string("out") + std::to_string(i) + "." + format).apply());

            return CompositeEncodedLightField::create(std::move(decodes));*/
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
            auto columns = EquirectangularTiledLightField<ColorSpace>::columns, rows = EquirectangularTiledLightField<ColorSpace>::rows;
            height = EquirectangularTiledLightField<ColorSpace>::height, width = EquirectangularTiledLightField<ColorSpace>::width;
            auto dinterval = (double)interval.numerator() / interval.denominator();

            std::string cwd = getcwd(nullptr, 0);
            std::string command{std::string("/home/bhaynes/projects/visualcloud/stitch.sh '") + cwd + "' " + std::to_string(height) + " " + std::to_string(width) + " " + std::to_string(rows) + " " + std::to_string(columns) + " " + std::to_string(dinterval)};
            system(command.c_str());

            return SingletonEncodedLightField::create("stitched.hevc");
        }

        template class EquirectangularTiledLightField<YUVColorSpace>;
        template class StitchedLightField<YUVColorSpace>;
    } // namespace physical
} // namespace visualcloud

