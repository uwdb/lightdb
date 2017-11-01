#include "Physical.h"
#include "Operators.h"
#include "TileVideoEncoder.h"
#include "CropTranscoder.h"
#include "Configuration.h"
#include <glog/logging.h>
#include <optional>
#include <unistd.h>

namespace visualcloud {
    namespace physical {
        template<typename ColorSpace>
        void EquirectangularTiledLightField<ColorSpace>::hardcode_hack(const unsigned int framerate, const unsigned int gop, const unsigned int height, const unsigned int width, const unsigned int rows, const unsigned int columns, const unsigned int max_bitrate, const std::string &decode_format, const std::string &encode_format) {
            //EquirectangularTiledLightField<ColorSpace>::framerate = framerate;
            EquirectangularTiledLightField<ColorSpace>::gop = gop;
            //EquirectangularTiledLightField<ColorSpace>::height = height;
            //EquirectangularTiledLightField<ColorSpace>::width = width;
            //EquirectangularTiledLightField<ColorSpace>::rows = rows;
            //EquirectangularTiledLightField<ColorSpace>::columns = columns;
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
            //TODO ignoring format parameter...
            LOG(INFO) << "Executing tiling physical operator with " << rows_ << " rows and " << columns_ << " columns";

            //auto framerate = 30u;
            size_t gop = video_.metadata().framerate.numerator() * time_ / video_.metadata().framerate.denominator();
            auto low_bitrate = 50u, high_bitrate = 5000u*1024;

            auto start = std::chrono::steady_clock::now();
            GPUContext context(0);

            //context.AttachToThread();
            DecodeConfiguration decodeConfiguration{video_.metadata().width, video_.metadata().height, video_.metadata().framerate, cudaVideoCodec_H264};
            std::vector<EncodeConfiguration> encodeConfigurations(
                    rows() * columns(),
                    EncodeConfiguration{video_.metadata().height/rows(), video_.metadata().width/columns(),
                                        NV_ENC_HEVC, video_.metadata().framerate, gop, low_bitrate, NV_ENC_PARAMS_RC_CBR});
            encodeConfigurations[1].bitrate = high_bitrate;

            TileVideoEncoder tiler(context, decodeConfiguration, encodeConfigurations, rows(), columns());

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
            LOG(INFO) << "Tile operator initialization took " << elapsed.count() << "ms";


            FileDecodeReader reader(video_.filename());
            std::vector<std::shared_ptr<EncodeWriter>> writers;

            for(auto i = 0; i < rows() * columns(); i++)
                writers.emplace_back(std::make_shared<SegmentedMemoryEncodeWriter>(tiler.api(), encodeConfigurations[0]));
                //writers.emplace_back(std::make_shared<FileEncodeWriter>(tiler.api(), std::string("out") + std::to_string(i)));

            tiler.tile(reader, writers);

            std::vector<std::shared_ptr<bytestring>> decodes{};
            //std::vector<std::unique_ptr<std::istream>> decodes{};
            for(auto i = 0; i < rows_ * columns_; i++)
                //decodes.emplace_back(bytestring{});
                decodes.emplace_back(std::make_shared<bytestring>(dynamic_cast<SegmentedMemoryEncodeWriter*>(writers[i].get())->buffer()));
                //decodes.emplace_back(std::make_unique<std::ifstream>(std::string("out") + std::to_string(i)));

            return CompositeMemoryEncodedLightField::create(decodes);
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
            //auto height = 0u, width = 0u;

            //TODO broken; this doesn't take tile coordinates into account
            /*for(auto &video: videos_) {
                auto resolution = video_.frames().resolution();
                height += resolution.second;
                width += resolution.first;
            }*/

            LOG(INFO) << "Hardcoded tile resolution; this will break for other tilings.";
            LOG(INFO) << "Wonky GOP/time interval splitting";
            //auto columns = EquirectangularTiledLightField<ColorSpace>::columns, rows = EquirectangularTiledLightField<ColorSpace>::rows;
            auto columns = 4, rows = 4;
            //auto height = 1920, width = 3840;
            auto height = videos_[0]->metadata().height * rows, width = videos_[0]->metadata().width * columns;
            //height = EquirectangularTiledLightField<ColorSpace>::height, width = EquirectangularTiledLightField<ColorSpace>::width;
            auto dinterval = (double)interval.numerator() / interval.denominator();

            std::string cwd = getcwd(nullptr, 0);
            std::string command{std::string("/home/bhaynes/projects/visualcloud/stitch.sh '") + cwd + "' " + std::to_string(height) + " " + std::to_string(width) + " " + std::to_string(rows) + " " + std::to_string(columns) + " " + std::to_string(dinterval)};
            system(command.c_str());

            return SingletonFileEncodedLightField::create("stitched.hevc");
        }

        template<typename ColorSpace>
        EncodedLightField EquirectangularCroppedLightField<ColorSpace>::apply(const std::string &format) {
            LOG(INFO) << "Executing ER cropping physical operator";

            size_t gop = video_.metadata().framerate.numerator() / video_.metadata().framerate.denominator();
            auto bitrate = 500*1024;
            auto encodeCodec = format == "h264" ? NV_ENC_H264 : NV_ENC_HEVC; //TODO what about others?
            auto phiratio = phi_.end / AngularRange::PhiMax.end;
            auto thetaratio = theta_.end / AngularRange::ThetaMax.end;
            auto top = std::lround((phi_.start / AngularRange::PhiMax.end) * video_.metadata().height),
                 bottom = std::lround((phi_.end / AngularRange::PhiMax.end) * video_.metadata().height),
                 left = std::lround((theta_.start / AngularRange::ThetaMax.end) * video_.metadata().width),
                 right = std::lround((theta_.end / AngularRange::ThetaMax.end) * video_.metadata().width);

            auto start = std::chrono::steady_clock::now();
            GPUContext context(0);

            //context.AttachToThread();
            DecodeConfiguration decodeConfiguration{video_.metadata().width, video_.metadata().height, video_.metadata().framerate, cudaVideoCodec_H264};
            //TODO why CBR?
            EncodeConfiguration encodeConfiguration{bottom - top, right - left,
                                                    NV_ENC_HEVC, video_.metadata().framerate, gop, bitrate, NV_ENC_PARAMS_RC_CBR};

            CropTranscoder cropper(context, decodeConfiguration, encodeConfiguration);

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
            LOG(INFO) << "Crop initialization took " << elapsed.count() << "ms";

            FileDecodeReader reader(video_.filename());
            SegmentedMemoryEncodeWriter writer{cropper.encoder().api(), encodeConfiguration};

            cropper.crop(reader, writer, top, left);

            auto decode = std::make_shared<bytestring>(writer.buffer());

            return SingletonMemoryEncodedLightField::create(decode);
        }

        template class EquirectangularTiledLightField<YUVColorSpace>;
        template class StitchedLightField<YUVColorSpace>;
        template class EquirectangularCroppedLightField<YUVColorSpace>;
    } // namespace physical
} // namespace visualcloud

