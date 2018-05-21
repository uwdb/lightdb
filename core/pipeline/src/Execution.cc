#include "Execution.h"
#include "PhysicalOperators.h"
#include "Encoding.h"
#include "Display.h"

namespace lightdb {
    namespace pipeline {
        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applyTiling(LightFieldReference lightfield, const std::string &format) {
            try {
                return {lightdb::physical::EquirectangularTiledLightField<YUVColorSpace>(lightfield).apply(format)};
            } catch(std::invalid_argument) {
                return {};
            }
        }

        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applyStitching(LightFieldReference lightfield, number temporalInterval=0) {
            try {
                return {lightdb::physical::StitchedLightField<YUVColorSpace>(lightfield).apply(temporalInterval)};
            } catch(std::invalid_argument) {
                //TODO just name these things, rather than relying on dynamic casting
                auto *partitioning = dynamic_cast<const logical::PartitionedLightField*>(&*lightfield);
                auto *discrete = dynamic_cast<const logical::DiscretizedLightField*>(&*lightfield);
                auto *interpolated = dynamic_cast<const logical::InterpolatedLightField*>(&*lightfield);

                if(partitioning != nullptr && partitioning->dimension() == Dimension::Time &&
                        lightfield->parents().size() == 1)
                    return applyStitching(lightfield->parents().at(0), partitioning->interval());
                //TODO this is mega broken; find the composite
                else if(discrete != nullptr || interpolated != nullptr)
                    return applyStitching(lightfield->parents().at(0), temporalInterval);
                else
                    return {};
            }
        }

        //TODO what is this, it's just a copy of the applyStitchingFunction?  Probably remove(?)
        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applyNaiveStitching(LightFieldReference lightfield, number temporalInterval=0) {
            try {
                return {lightdb::physical::StitchedLightField<YUVColorSpace>(lightfield).apply(temporalInterval)};
            } catch(std::invalid_argument) {
                //TODO just name these things, rather than relying on dynamic casting
                auto *partitioning = dynamic_cast<const logical::PartitionedLightField*>(&*lightfield);
                auto *discrete = dynamic_cast<const logical::DiscretizedLightField*>(&*lightfield);
                auto *interpolated = dynamic_cast<const logical::InterpolatedLightField*>(&*lightfield);

                if(partitioning != nullptr && partitioning->dimension() == Dimension::Time &&
                        lightfield->parents().size() == 1)
                    return applyStitching(lightfield->parents().at(0), partitioning->interval());
                    //TODO this is mega broken; find the composite
                else if(discrete != nullptr || interpolated != nullptr)
                    return applyStitching(lightfield->parents().at(0), temporalInterval);
                else
                    return {};
            }
        }

        //TODO what is this, it's just a copy of the applyStitchingFunction?  Probably remove(?)
        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applyBinaryOverlayUnion(LightFieldReference lightfield, const std::string format) {
            auto *composite = dynamic_cast<const logical::CompositeLightField*>(&*lightfield);
            if(composite == nullptr || composite->parents().size() != 2)
                return {};

            auto *left = dynamic_cast<const logical::PanoramicVideoLightField*>(&*composite->parents()[0]);
            auto *right = dynamic_cast<const logical::PanoramicVideoLightField*>(&*composite->parents()[1]);

            if(left != nullptr && right != nullptr &&
                    static_cast<const LightField*>(left)->volume().bounding() == static_cast<const LightField*>(right)->volume().bounding()) {
                //TODO this is just broken, need a binary union transcoder
                return {lightdb::physical::BinaryUnionTranscodedLightField(*left, *right, Overlay(YUVColor::red())).apply(format)};
            }
            else
                return {};
        }

        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applySelfUnion(LightFieldReference lightfield, const std::string &format) {
            auto *composite = dynamic_cast<const logical::CompositeLightField*>(&*lightfield);
            if(composite == nullptr || composite->parents().size() != 2)
                return {};

            auto *left = dynamic_cast<logical::PanoramicVideoLightField*>(&*composite->parents()[0]);
            auto *right = dynamic_cast<logical::PanoramicVideoLightField*>(&*composite->parents()[1]);

            if(left != nullptr && right != nullptr &&
                    static_cast<const LightField*>(left)->volume().bounding() == static_cast<const LightField*>(right)->volume().bounding() &&
               left->filename() == right->filename() &&
               left->metadata().codec().name() == format) {
                auto lf = composite->parents()[0];
                auto sp = static_cast<const std::shared_ptr<LightField>>(lf);
                auto vlf = std::static_pointer_cast<logical::PanoramicVideoLightField>(sp);
                auto elf = std::static_pointer_cast<EncodedLightFieldData>(vlf);

                return elf;
            }
            else
                return {};
        }

        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applyIdentitySelectTranscode(LightFieldReference lightfield, const std::string &format) {
            auto *subset = dynamic_cast<logical::SubsetLightField*>(&*lightfield);
            auto *video = subset != nullptr ? dynamic_cast<logical::PanoramicVideoLightField*>(&*lightfield->parents()[0]) : nullptr;

            if(subset != nullptr && video != nullptr &&
                    subset->volume().components().size() == 1 &&
                    subset->volume().components()[0] == static_cast<const LightField*>(video)->volume().bounding() &&
                    video->metadata().codec().name() == format) {
                auto lf = lightfield->parents()[0];
                auto sp = static_cast<const std::shared_ptr<LightField>>(lf);
                auto vlf = std::static_pointer_cast<logical::PanoramicVideoLightField>(sp);
                auto elf = std::static_pointer_cast<EncodedLightFieldData>(vlf);

                return elf;
            }
            else
                return {};
        }

        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applyIdentityTranscode(LightFieldReference lightfield, const std::string &format) {
            auto *video = dynamic_cast<logical::PanoramicVideoLightField*>(&*lightfield);
            if(video != nullptr && video->metadata().codec().name() == format) {
                auto sp = static_cast<const std::shared_ptr<LightField>>(lightfield);
                auto vlf = std::static_pointer_cast<logical::PanoramicVideoLightField>(sp);
                auto elf = std::static_pointer_cast<EncodedLightFieldData>(vlf);

                return elf;
            }
            else
                return {};
        }

        static std::optional<EncodedLightField> applyColorFromPointConstantTranscode(LightFieldReference lightfield, const std::string &format) {
            auto *constant = dynamic_cast<logical::ConstantLightField*>(&*lightfield);
            if(constant != nullptr && format == "YUV" && constant->volume().components().size() == 1 && constant->volume().components()[0].is_point()) {
                auto data = std::make_shared<bytestring>(constant->color());
                return SingletonMemoryEncodedLightField::create(data, constant->volume().components()[0]);
            }
            else
                return {};
        }

        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applyTranscode(LightFieldReference lightfield, const std::string &format) {
            auto *video = dynamic_cast<logical::PanoramicVideoLightField*>(&*lightfield);
            if(video != nullptr && video->metadata().codec().name() != format)
                return std::optional<EncodedLightField>{lightdb::physical::EquirectangularTranscodedLightField<YUVColorSpace>(*video, Identity()).apply(format)};
            else
                return {};
        }

        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applyTemporalPartitonedTranscode(LightFieldReference lightfield, const std::string &format) {
            auto *partitioning = dynamic_cast<const logical::PartitionedLightField*>(&*lightfield);
            auto *video = partitioning != nullptr && partitioning->parents().size() == 1 ? dynamic_cast<logical::PanoramicVideoLightField*>(&*partitioning->parents()[0]) : nullptr;

            if(partitioning != nullptr && video != nullptr && video->metadata().codec().name() != format && static_cast<LightField*>(video)->volume().components().size() > 0)
                return {lightdb::physical::TemporalPartitionedEquirectangularTranscodedLightField<YUVColorSpace>(*partitioning, *video, Identity()).apply(format)};
            else
                return {};
        }

        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applyTransformTranscode(LightFieldReference lightfield, const std::string &format) {
            if(lightfield->parents().size() != 1)
                return {};

            auto *transform = dynamic_cast<logical::TransformedLightField*>(&*lightfield);
            auto *video = dynamic_cast<logical::PanoramicVideoLightField*>(&*lightfield->parents()[0]);
            if(transform != nullptr && video != nullptr &&
               transform->functor()->hasFrameTransform())
                return {lightdb::physical::EquirectangularTranscodedLightField<YUVColorSpace>(*video, *transform->functor()).apply(format)};
            else
                return {};
        }

        /*
        template<typename ColorSpace>
        static std::optional<EncodedLightField> applyRotatedStitching(LightFieldReference<ColorSpace> lightfield, const std::string &format, lightdb::rational temporalInterval=0) {
            //if(lightfield->provenance().size() != 1)
            //    return {};

            //auto *transform = dynamic_cast<TransformedLightField<ColorSpace>*>(&*lightfield);
            //auto *video = dynamic_cast<PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*>(&*lightfield->provenance()[0]);
            //if(transform != nullptr && video != nullptr &&
            //   transform->functor().hasFrameTransform())
            //    return lightdb::physical::EquirectangularTranscodedLightField<ColorSpace>(*video, transform->functor()).apply(format);
            //else
            //    return {};

            print_plan(lightfield);

            auto *composite = dynamic_cast<const CompositeLightField<ColorSpace>*>(&*lightfield);

            if(composite == nullptr || composite->provenance().size() != 2)
                return {};

            try {
                auto *vleft = dynamic_cast<const PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*>(&*composite->provenance()[0]);
                auto *rright = dynamic_cast<const RotatedLightField<ColorSpace>*>(&*composite->provenance()[1]);
                auto *vright = rright != nullptr ? dynamic_cast<const PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*>(&*rright->provenance()[0]) : nullptr;

                if(rright == nullptr || rright->volumes().size() != 1)
                    return {};
                else if(vleft->volumes()[0].theta.start != 0 ||
                        rright->volumes()[0].theta.end - AngularRange::ThetaMax.end > 0.00000001||
                        vleft->volumes()[0].theta.end - rright->volumes()[0].theta.start > 0.00000001)
                    return {};
                else if(vleft->volumes()[0].phi.start != 0 ||
                        vleft->volumes()[0].phi.end - AngularRange::PhiMax.end > 0.00000001 ||
                        rright->volumes()[0].phi.start != 0 ||
                        rright->volumes()[0].phi.end - AngularRange::PhiMax.end > 0.00000001)
                    return {};
                else
                    return {lightdb::physical::StitchedLightField<ColorSpace>(lightfield).apply(temporalInterval)};

                //auto *sleft = dynamic_cast<const SubsetLightField<ColorSpace>*>(&*composite->provenance()[0]);
                //auto *vleft = dynamic_cast<const PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*>(&*sleft->provenance()[0]);

                //auto *rright = dynamic_cast<const RotatedLightField<ColorSpace>*>(&*composite->provenance()[1]);
                //auto *sright = dynamic_cast<const SubsetLightField<ColorSpace>*>(&*rright->parents()[0]);
                //auto *vright = dynamic_cast<const PanoramicVideoLightField<EquirectangularGeometry, ColorSpace>*>(&*sright->provenance()[0]);

                //if(sleft->volumes().size() != 1 || sright->volumes().size() != 1)
                //    return {};
                //else if(sleft->volumes()[0].x != sright->volumes()[0].x ||
                //        sleft->volumes()[0].y != sright->volumes()[0].y ||
                //        sleft->volumes()[0].z != sright->volumes()[0].z ||
                //        sleft->volumes()[0].t != sright->volumes()[0].t)
                //    return {};

                //else if(sleft->volumes()[0].theta.start != 0 ||
                //        rright->volumes()[0].theta.end - AngularRange::ThetaMax.end > 0.00000001||
                //        sleft->volumes()[0].theta.end - rright->volumes()[0].theta.start > 0.00000001)
                //    return {};
                //else if(sleft->volumes()[0].phi.start != 0 ||
                //        sright->volumes()[0].phi.end - AngularRange::PhiMax.end > 0.00000001 ||
                //        rright->volumes()[0].phi.start != 0 ||
                //        rright->volumes()[0].phi.end - AngularRange::PhiMax.end > 0.00000001)
                //    return {};
                //else
                //    return {lightdb::physical::StitchedLightField<ColorSpace>(lightfield).apply(temporalInterval)};
            } catch(std::invalid_argument) {
                //TODO awful
                return {};
            }
        }
*/
        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applySelection(LightFieldReference lightfield, const std::string &format) {
            if(lightfield->parents().size() != 1)
                return {};

            auto *subset = dynamic_cast<logical::SubsetLightField*>(&*lightfield);
            auto *video = dynamic_cast<logical::PanoramicVideoLightField*>(&*lightfield->parents()[0]);
            if(subset != nullptr && video != nullptr &&
                    subset->volume().components().size() == 1 &&
                    subset->volume().components()[0].x().empty() &&
                    subset->volume().components()[0].y().empty() &&
                    subset->volume().components()[0].z().empty()) {
                auto elf = lightdb::physical::EquirectangularCroppedLightField<YUVColorSpace>(*video, subset->volume().components()[0].theta(), subset->volume().components()[0].phi(), subset->volume().components()[0].t()).apply(format);
                return elf;
            }
            else
                return {};
        }

        //template<typename ColorSpace>
        static std::optional<EncodedLightField> applyLightFieldToVideoSelection(LightFieldReference lightfield, const std::string &format) {
            if(lightfield->parents().size() != 1)
                return {};

            auto *subset = dynamic_cast<logical::SubsetLightField*>(&*lightfield);
            auto *planar = dynamic_cast<logical::PlanarTiledVideoLightField*>(&*lightfield->parents()[0]);
            if(subset != nullptr && planar != nullptr &&
               subset->volume().components().size() == 1 &&
               subset->volume().components()[0].x().empty() &&
               subset->volume().components()[0].y().empty() &&
               subset->volume().components()[0].z().empty() &&
                    subset->volume().components()[0].z().start() == 0) {
                auto elf = lightdb::physical::PlanarTiledToVideoLightField(*planar, subset->volume().components()[0].x().start(), subset->volume().components()[0].y().start(), subset->volume().components()[0].theta(), subset->volume().components()[0].phi()).apply(format);
                return elf;
            }
            else
                return {};
        }

        EncodedLightField execute(LightFieldReference lightfield, const std::string &format) {
            print_plan(lightfield);

            //auto volumes = lightfield->volumes();
            //auto vl = volumes.size();

            std::optional<EncodedLightField> result;
            if((result = applyColorFromPointConstantTranscode(lightfield, format)).has_value())
                return result.value();
            else if((result = applyIdentityTranscode(lightfield, format)).has_value())
                return result.value();
            else if((result = applyIdentitySelectTranscode(lightfield, format)).has_value())
                return result.value();
            else if((result = applySelfUnion(lightfield, format)).has_value())
                return result.value();
            else if((result = applyTemporalPartitonedTranscode(lightfield, format)).has_value())
                return result.value();
            else if((result = applyTranscode(lightfield, format)).has_value())
                return result.value();
            else if((result = applyTiling(lightfield, format)).has_value())
                return result.value();
            else if((result = applyStitching(lightfield)).has_value())
                return result.value();
            else if((result = applyNaiveStitching(lightfield)).has_value()) // Should come after fast stitching, I guess
                return result.value();
            else if((result = applyBinaryOverlayUnion(lightfield, format)).has_value())
                return result.value();
            //else if((result = applyRotatedStitching(lightfield, format)).has_value())
            //    return result.value();
            else if((result = applySelection(lightfield, format)).has_value())
                return result.value();
            else if((result = applyTransformTranscode(lightfield, format)).has_value())
                return result.value();
            else if((result = applyLightFieldToVideoSelection(lightfield, format)))
                return result.value();
            else
                throw std::runtime_error("Unable to execute query");

            //lightdb::physical::EquirectangularTiledLightField<ColorSpace> foo(lightfield);
            //return foo.apply(format);
        }

        //template EncodedLightField execute<YUVColorSpace>(const LightFieldReference, const std::string&);
    } // namespace pipeline
} // namespace lightdb
