#ifndef VISUALCLOUD_PHYSICAL_H
#define VISUALCLOUD_PHYSICAL_H

#include "LightField.h"
#include "Encoding.h"
#include "TileVideoEncoder.h" //TODO can remove this after hacks addressed
#include <tuple>
#include <stdexcept>

namespace visualcloud {
    namespace physical {

        template<typename ColorSpace>
        class EquirectangularTiledLightField: public LightField<ColorSpace> {
        public:
            EquirectangularTiledLightField(LightFieldReference<ColorSpace> &field)
                : EquirectangularTiledLightField(field_, get_dimensions(&*field))
            { }

            const std::vector<LightFieldReference<ColorSpace>> provenance() const override { return {field_}; }
            const ColorSpace colorSpace() const override { return ColorSpace::Instance; }
            const std::vector<Volume> volumes() const override { return field_->volumes(); }
            inline const typename ColorSpace::Color value(const Point6D &point) const override { return field_->value(point); }

            EncodedLightField apply(const std::string&);

            static void hardcode_hack(const unsigned int framerate, const unsigned int gop, const unsigned int height, const unsigned int width, const unsigned int rows, const unsigned int columns, const unsigned int max_bitrate, const std::string &intermediate_format, const std::string &output_format);

        private:
            using metadata = std::tuple<size_t, size_t, PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>&>;

            EquirectangularTiledLightField(LightFieldReference<ColorSpace> &field, const metadata data)
                : field_(field), rows_(std::get<0>(data)), columns_(std::get<1>(data)), video_((std::get<2>(data)))
            { }

            //TODO hacks...
            static double hack_divide(const double left, const visualcloud::rational &right) {
                //TODO oh noes...
                return left / ((double)right.numerator() / (double)right.denominator()) + 0.5;
            }
            static unsigned int framerate, gop, height, width;
            static EncodeConfiguration *encodeConfiguration;
            static DecodeConfiguration *decodeConfiguration;
            static GPUContext *context;
            static TileVideoEncoder *tiler;


            static metadata get_dimensions(LightField<ColorSpace>* field, size_t rows=1, size_t columns=1) {
                auto *partitioner = dynamic_cast<const PartitionedLightField<ColorSpace>*>(field);
                auto *video = dynamic_cast<PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*>(field);
                auto *child = field->provenance().size() == 1 ? &*field->provenance().at(0) : nullptr;
                // Don't cast for every prohibited type
                auto *discrete = dynamic_cast<DiscreteLightField<ColorSpace>*>(field);

                if(video != nullptr) {
                    if(rows > 1 || columns > 1)
                        return {rows, columns, *video};
                    else {
                        LOG(WARNING) << "Attempt to perform 1x1 tiling; use transcode instead";
                        throw std::invalid_argument("Attempt to perform 1x1 tiling; ignore (or use transcode if format changed) instead");
                    }
                } else if(partitioner != nullptr && partitioner->dimension() == Dimension::Theta)
                    return get_dimensions(child, rows, hack_divide(AngularRange::ThetaMax.end, partitioner->interval()));
                else if(partitioner != nullptr && partitioner->dimension() == Dimension::Phi)
                    return get_dimensions(child, hack_divide(AngularRange::PhiMax.end, partitioner->interval()), columns);
                // TODO prohibit other intermediating field types...
                // TODO volume may not be pointwise spatial...
                else if(child != nullptr && discrete == nullptr)
                    return get_dimensions(child, rows, columns);
                else {
                    LOG(WARNING) << "Attempt to tile field not backed by logical PanoramicVideoLightField";
                    throw std::invalid_argument("Query not backed by logical PanoramicVideoLightField");
                }
            }

            LightFieldReference<ColorSpace> field_;
            const size_t rows_, columns_;
            const PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>& video_;
        };

        template<typename ColorSpace>
        class StitchedLightField: public LightField<ColorSpace> {
        public:
            StitchedLightField(const LightFieldReference<ColorSpace> &field)
                    : field_(field), videos_(get_tiles(field))
            { }

            const std::vector<LightFieldReference<ColorSpace>> provenance() const override { return {field_}; }
            const ColorSpace colorSpace() const override { return ColorSpace::Instance; }
            const std::vector<Volume> volumes() const override { return field_->volumes(); }
            inline const typename ColorSpace::Color value(const Point6D &point) const override { return field_->value(point); }

            EncodedLightField apply();
            EncodedLightField apply(const visualcloud::rational &temporalInterval);

        private:
            static std::vector<PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*> get_tiles(const LightFieldReference<ColorSpace>& field) {
                auto *composite = dynamic_cast<const CompositeLightField<ColorSpace>*>(&*field);
                std::vector<PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*> videos;

                if(composite == nullptr)
                    throw std::invalid_argument("Plan root was not a composite.");

                for(auto &child: field->provenance()) {
                    LightField<ColorSpace> *cf = &*child;
                    auto *video = dynamic_cast<PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*>(cf);
                    if(video == nullptr)
                        throw std::invalid_argument("Composite child was not a video.");
                    videos.push_back(video);
                }

                return videos;
            }

            const LightFieldReference<ColorSpace> field_;
            const std::vector<PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*> videos_;
        };

        //TODO hacks
        template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::framerate = 0;
        template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::gop = 0;
        template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::height = 0;
        template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::width = 0;
        template<typename ColorSpace> EncodeConfiguration* EquirectangularTiledLightField<ColorSpace>::encodeConfiguration = nullptr;
        template<typename ColorSpace> DecodeConfiguration* EquirectangularTiledLightField<ColorSpace>::decodeConfiguration = nullptr;
        template<typename ColorSpace> GPUContext* EquirectangularTiledLightField<ColorSpace>::context = nullptr;
        template<typename ColorSpace> TileVideoEncoder* EquirectangularTiledLightField<ColorSpace>::tiler = nullptr;
    } // namespace physical
} // namespace visualcloud

#endif //VISUALCLOUD_PHYSICAL_H
