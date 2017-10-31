#include "Execution.h"
#include "Physical.h"
#include "Display.h"

namespace visualcloud {
    namespace pipeline {
        template<typename ColorSpace>
        static std::optional<EncodedLightField> applyTiling(LightFieldReference <ColorSpace> lightfield, const std::string &format) {
            try {
                return visualcloud::physical::EquirectangularTiledLightField<ColorSpace>(lightfield).apply(format);
            } catch(std::invalid_argument) {
                return {};
            }
        }

        template<typename ColorSpace>
        static std::optional<EncodedLightField> applyStitching(LightFieldReference<ColorSpace> lightfield, visualcloud::rational temporalInterval=0) {
            try {
                return visualcloud::physical::StitchedLightField<ColorSpace>(lightfield).apply(temporalInterval);
            } catch(std::invalid_argument) {
                //TODO just name these things, rather than relying on dynamic casting
                auto *partitioning = dynamic_cast<const PartitionedLightField<ColorSpace>*>(&*lightfield);
                auto *discrete = dynamic_cast<const DiscretizedLightField<ColorSpace>*>(&*lightfield);
                auto *interpolated = dynamic_cast<const InterpolatedLightField<ColorSpace>*>(&*lightfield);

                if(partitioning != nullptr && partitioning->dimension() == Dimension::Time && lightfield->provenance().size() == 1)
                    return applyStitching(lightfield->provenance().at(0), partitioning->interval());
                //TODO this is mega broken; find the composite
                else if(discrete != nullptr || interpolated != nullptr)
                    return applyStitching(lightfield->provenance().at(0), temporalInterval);
                else
                    return {};
            }
        }

        template<typename ColorSpace>
        static std::optional<EncodedLightField> applyIdentityTranscode(LightFieldReference<ColorSpace> lightfield, const std::string &format) {
            auto *video = dynamic_cast<PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*>(&*lightfield);
            if(video != nullptr && video->metadata().codec == format) {
                auto sp = static_cast<const std::shared_ptr<LightField<ColorSpace>>>(lightfield);
                auto vlf = std::static_pointer_cast<PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>>(sp);
                auto elf = std::static_pointer_cast<EncodedLightFieldData>(vlf);
                //auto vlf = dynamic_cast<const std::shared_ptr<PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>>*>(&sp);
                //auto *elf = reinterpret_cast<const std::shared_ptr<EncodedLightFieldData>*>(vlf);

                return elf;
            }
            else
                return {};
        }

        template<typename ColorSpace>
        EncodedLightField execute(LightFieldReference <ColorSpace> lightfield, const std::string &format) {
            //print_plan(lightfield);

            //auto volumes = lightfield->volumes();
            //auto vl = volumes.size();

            std::optional<EncodedLightField> result;
            if((result = applyIdentityTranscode(lightfield, format)).has_value())
                return result.value();
            else if((result = applyTiling(lightfield, format)).has_value())
                return result.value();
            else if((result = applyStitching(lightfield)).has_value())
                return result.value();
            else
                throw std::runtime_error("Unable to execute query");

            //visualcloud::physical::EquirectangularTiledLightField<ColorSpace> foo(lightfield);
            //return foo.apply(format);
        }

        template EncodedLightField execute<YUVColorSpace>(const LightFieldReference <YUVColorSpace>, const std::string&);
    } // namespace pipeline
} // namespace visualcloud
