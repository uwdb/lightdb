#ifndef LIGHTDB_OLDPHYSICALOPERATORS_H
#define LIGHTDB_OLDPHYSICALOPERATORS_H

#include "TileVideoEncoder.h" //TODO remove when physical ops are split out
#include "OldModel.h"
#include "../../functions/include/Greyscale.h"
#include "Functor.h"

namespace lightdb::physical {
    class unaryfunctor {};
    class binaryfunctor {};

    template<typename ColorSpace>
    class EquirectangularTiledLightField: public LightField {
    public:
        EquirectangularTiledLightField(LightFieldReference &field)
                : EquirectangularTiledLightField(field_, get_dimensions(&*field))
        { }

        const std::vector<LightFieldReference>& parents() const noexcept override { return fieldvec; }
        const lightdb::ColorSpace colorSpace() const noexcept override { return ColorSpace::instance(); }
        //const CompositeVolume volume() const override { return field_->volume(); }
        const unsigned int rows() const { return rows_; }
        const unsigned int columns() const { return columns_; }
//            inline const YUVColor value(const Point6D &point) const override { return field_->value(point); }

        EncodedLightField apply(const std::string&);

        static void hardcode_hack(const unsigned int framerate, const unsigned int gop, const unsigned int height, const unsigned int width, const unsigned int rows, const unsigned int columns, const unsigned int max_bitrate, const std::string &intermediate_format, const std::string &output_format);
        //TODO hacks...
        static double hack_divide(const double left, const lightdb::rational &right) {
            //TODO oh noes...
            return left / ((double)right.numerator() / (double)right.denominator()) + 0.5;
        }
        static double hack_divide(const double left, const double &right) {
            //TODO oh noes...
            return left / right + 0.5;
        }
        static long double hack_divide(const number &left, const number &right) {
            //TODO oh noes...
            return (long double)(left / right + 0.5);
        }
        static unsigned int gop, max_bitrate; //framerate, height, width; //, rows, columns;
        static std::shared_ptr<EncodeConfiguration> encodeConfiguration;
        static std::shared_ptr<DecodeConfiguration> decodeConfiguration;
        static std::shared_ptr<GPUContext> context;
        static std::shared_ptr<TileVideoEncoder> tiler;
        static std::string decode_format, encode_format;
        static bool executed;
        //GPUContext context_; //TODO

        void accept(LightFieldVisitor &visitor) override { }

    private:
        using metadata = std::tuple<size_t, size_t, size_t, logical::PanoramicVideoLightField&>;

        EquirectangularTiledLightField(LightFieldReference &field, const metadata data)
                : LightField(field), //, field->volume()),
                  field_(field), rows_(std::get<0>(data)), columns_(std::get<1>(data)), time_(std::get<2>(data)), video_((std::get<3>(data))), fieldvec({field})
        //context_(0) //TODO context
        { }

        static metadata get_dimensions(LightField* field, size_t rows=1, size_t columns=1, size_t time=0) {
            auto *partitioner = dynamic_cast<const logical::PartitionedLightField*>(field);
            auto *video = dynamic_cast<logical::PanoramicVideoLightField*>(field);
            auto *child = field->parents().size() == 1 ? &*field->parents().at(0) : nullptr;
            // Don't cast for every prohibited type
            auto *discrete = dynamic_cast<logical::DiscreteLightField*>(field);

            if(video != nullptr) {
                if(rows > 1 || columns > 1)
                    return {rows, columns, time, *video};
                else {
                    LOG(WARNING) << "Attempt to perform 1x1 tiling; use transcode instead";
                    throw std::invalid_argument("Attempt to perform 1x1 tiling; ignore (or use transcode if format changed) instead");
                }
            } else if(partitioner != nullptr && partitioner->dimension() == Dimension::Theta)
                return get_dimensions(child, rows, hack_divide(ThetaRange::limits().end(), partitioner->interval()), time);
            else if(partitioner != nullptr && partitioner->dimension() == Dimension::Phi)
                return get_dimensions(child, hack_divide(PhiRange::limits().end(), partitioner->interval()), columns, time);
            else if(partitioner != nullptr && partitioner->dimension() == Dimension::Time)
                return get_dimensions(child, rows, columns, (unsigned int)partitioner->interval());
                // TODO prohibit other intermediating field types...
                // TODO volume may not be pointwise spatial...
            else if(child != nullptr && discrete == nullptr)
                return get_dimensions(child, rows, columns, time);
            else {
                LOG(WARNING) << "Attempt to tile field not backed by logical PanoramicVideoLightField";
                throw std::invalid_argument("Query not backed by logical PanoramicVideoLightField");
            }
        }

        LightFieldReference field_;
        const unsigned int rows_, columns_;
        const number time_;
        const logical::PanoramicVideoLightField& video_;
        std::vector<LightFieldReference> fieldvec;
    };

    template<typename ColorSpace>
    class StitchedLightField: public LightField {
    public:
        StitchedLightField(const LightFieldReference &field)
                : StitchedLightField(field, get_tiles(field))
        { }

        const std::vector<LightFieldReference>& parents() const noexcept override { return v; }
        const lightdb::ColorSpace colorSpace() const noexcept override { return ColorSpace::instance(); }
        //const CompositeVolume volume() const override { return field_->volume(); }
//            inline const YUVColor value(const Point6D &point) const override { return field_->value(point); }

        EncodedLightField apply();
        EncodedLightField apply(const number &temporalInterval);

        void accept(LightFieldVisitor &visitor) override { }

    private:
        StitchedLightField(const LightFieldReference &field,
                           const std::pair<std::vector<logical::PanoramicVideoLightField*>, std::vector<Volume>> &pair)
                : LightField(field), //field->volume()),
                  field_(field), videos_(pair.first), volumes_(pair.second), v({field})
        { }

        static std::pair<std::vector<logical::PanoramicVideoLightField*>, std::vector<Volume>> get_tiles(const LightFieldReference& field) {
            auto *composite = dynamic_cast<const logical::CompositeLightField*>(&*field);
            std::vector<logical::PanoramicVideoLightField*> videos;
            std::vector<Volume> volumes;

            if(composite == nullptr)
                throw std::invalid_argument("Plan root was not a composite.");

            for(auto &child: field->parents()) {
                LightField *cf = &*child;
                auto *video = dynamic_cast<logical::PanoramicVideoLightField*>(cf);
                auto *rotation = dynamic_cast<logical::RotatedLightField*>(cf);
                if(rotation != nullptr)
                    video = dynamic_cast<logical::PanoramicVideoLightField*>(&*rotation->parents()[0]);

                if(video == nullptr)
                    throw std::invalid_argument("Composite child was not a video.");
                else if(video->metadata().codec().name() != "hevc")
                    throw std::invalid_argument("Input video was not HEVC encoded.");

                videos.push_back(video);
                volumes.push_back(rotation != nullptr ? rotation->volume().components()[0] : static_cast<EncodedLightField>(video)->volume().components()[0]);
            }

            return std::make_pair(videos, volumes);
        }

        const LightFieldReference field_;
        const std::vector<logical::PanoramicVideoLightField*> videos_;
        const std::vector<Volume> volumes_;
        std::vector<LightFieldReference> v;
    };

    template<typename ColorSpace>
    class NaiveStitchedLightField: public LightField {
    public:
        NaiveStitchedLightField(const LightFieldReference &field)
                : NaiveStitchedLightField(field, get_tiles(field))
        { }

        const std::vector<LightFieldReference> parents() const override { return {field_}; }
        const ColorSpace colorSpace() const override { return ColorSpace::Instance; }
        const CompositeVolume volume() const override { return field_->volume(); }
//            inline const typename ColorSpace::Color value(const Point6D &point) const override { return field_->value(point); }

        EncodedLightField apply();

        void accept(LightFieldVisitor &visitor) override { }

    private:
        NaiveStitchedLightField(const LightFieldReference &field,
                                const std::pair<std::vector<logical::PanoramicVideoLightField*>,
                                        std::vector<Volume>> &pair)
                : field_(field), videos_(pair.first), volumes_(pair.second)
        { }

        static std::pair<std::vector<logical::PanoramicVideoLightField*>, std::vector<Volume>> get_tiles(const LightFieldReference& field) {
            auto *composite = dynamic_cast<const logical::CompositeLightField*>(&*field);
            std::vector<logical::PanoramicVideoLightField*> videos;
            std::vector<Volume> volumes;

            if(composite == nullptr)
                throw std::invalid_argument("Plan root was not a composite.");

            for(auto &child: field->parents()) {
                LightField *cf = &*child;
                auto *video = dynamic_cast<logical::PanoramicVideoLightField*>(cf);
                auto *rotation = dynamic_cast<logical::RotatedLightField*>(cf);
                if(rotation != nullptr)
                    video = dynamic_cast<logical::PanoramicVideoLightField*>(&*rotation->parents()[0]);

                if(video == nullptr)
                    throw std::invalid_argument("Composite child was not a video.");

                videos.push_back(video);
                volumes.push_back(rotation != nullptr ? rotation->volume().components()[0] : static_cast<EncodedLightField>(video)->volume().components()[0]);
            }

            return std::make_pair(videos, volumes);
        }

        const LightFieldReference field_;
        const std::vector<logical::PanoramicVideoLightField*> videos_;
        const std::vector<Volume> volumes_;
    };

    template<typename ColorSpace>
    class EquirectangularCroppedLightField: public LightField {
    public:
        EquirectangularCroppedLightField(const logical::PanoramicVideoLightField &video, ThetaRange theta, PhiRange phi, TemporalRange t)
                : LightField({}, static_cast<const LightField&>(video).volume(), video.colorSpace()), //TODO parents is incorrect
                  video_(video), theta_(theta), phi_(phi), t(t)
        { }

        std::vector<LightFieldReference> empty;

        const std::vector<LightFieldReference>& parents() const noexcept override { return empty; } //TODO incorrect
        const lightdb::ColorSpace colorSpace() const noexcept override { return ColorSpace::instance(); }
        //const CompositeVolume volume() const override { return video_.volume(); }
//            inline const typename ColorSpace::Color value(const Point6D &point) const override { return video_.value(point); }

        EncodedLightField apply(const std::string &format);

        void accept(LightFieldVisitor &visitor) override { }

    private:
        const logical::PanoramicVideoLightField& video_;
        const ThetaRange theta_;
        const PhiRange phi_;
        const TemporalRange t;
    };

    template<typename ColorSpace>
    class EquirectangularTranscodedLightField: public LightField {
    public:
        EquirectangularTranscodedLightField(const logical::PanoramicVideoLightField &video,
                                            unaryfunctor &functor)
                : LightField({}, static_cast<const LightField&>(video).volume(), video.colorSpace()), //TODO parents is incorrect
                  video_(video), functor_(functor)
        { }

        const std::vector<LightFieldReference>& parents() const noexcept override { return video_.parents(); } //TODO incorrect
        const lightdb::ColorSpace colorSpace() const noexcept override { return ColorSpace::instance(); }
        //const CompositeVolume volume() const override { return video_.volume(); }
/*            inline const YUVColor value(const Point6D &point) const override {
                return functor_(video_, point);
            }*/

        EncodedLightField apply(const std::string &format);

        void accept(LightFieldVisitor &visitor) override { }

    private:
        const logical::PanoramicVideoLightField& video_;
        unaryfunctor &functor_;
    };

    class PlanarTiledToVideoLightField: public LightField {
    public:
        PlanarTiledToVideoLightField(const logical::PlanarTiledVideoLightField &video,
                                     const number x, const number y, const ThetaRange &theta, const PhiRange &phi)
                : LightField({}, static_cast<const LightField&>(video).volume(), video.colorSpace()), //TODO parents is incorrect
                  video_(video), x_(x), y_(y), theta_(theta), phi_(phi)
        { }

        const std::vector<LightFieldReference>& parents() const noexcept override { return video_.parents(); } //TODO incorrect
        const ColorSpace colorSpace() const noexcept override { return YUVColorSpace::instance(); }
        //const CompositeVolume volume() const override { return video_.volume(); }
/*            inline const YUVColor value(const Point6D &point) const override {
                return video_.value(point);
            }*/

        EncodedLightField apply(const std::string &format);

        void accept(LightFieldVisitor &visitor) override { }

    private:
        const logical::PlanarTiledVideoLightField& video_;
        const number x_, y_;
        const ThetaRange theta_;
        const PhiRange phi_;
    };

    template<typename ColorSpace>
    class TemporalPartitionedEquirectangularTranscodedLightField: public LightField {
    public:
        TemporalPartitionedEquirectangularTranscodedLightField(
                const logical::PartitionedLightField &partitioning,
                const logical::PanoramicVideoLightField &video,
                unaryfunctor &functor)
                : LightField({}, static_cast<const LightField&>(video).volume(), video.colorSpace()), //TODO parents is incorrect
                  partitioning_(partitioning), video_(video), functor_(functor)
        { }

        const std::vector<LightFieldReference>& parents() const noexcept override { return video_.parents(); } //TODO incorrect
        const lightdb::ColorSpace colorSpace() const noexcept override { return ColorSpace::instance(); }
        //const CompositeVolume volume() const override { return video_.volume(); }
/*            inline const typename ColorSpace::Color value(const Point6D &point) const override {
                return functor_(video_, point);
            }*/

        EncodedLightField apply(const std::string &format);

        void accept(LightFieldVisitor &visitor) override { }

    private:
        const logical::PartitionedLightField& partitioning_;
        const logical::PanoramicVideoLightField& video_;
        unaryfunctor &functor_;
    };

    class BinaryUnionTranscodedLightField: public LightField {
    public:
        BinaryUnionTranscodedLightField(const logical::PanoramicVideoLightField &left,
                                        const logical::PanoramicVideoLightField &right,
                                        const binaryfunctor &functor)
                : LightField({}, static_cast<const LightField&>(left).volume(), left.colorSpace()), //TODO parents+colorspace is incorrect
                  left_(left), right_(right), functor_(functor)
        { }

        const std::vector<LightFieldReference>& parents() const noexcept override { return left_.parents(); } //TODO incorrect
        const lightdb::ColorSpace colorSpace() const noexcept override { return YUVColorSpace::instance(); }
        //const CompositeVolume volume() const override { return left_.volume(); } //TODO incorrect
/*            inline const YUVColor value(const Point6D &point) const override {
                return left_.value(point); //TOOD incorrect
            }*/

        EncodedLightField apply(const std::string &format);

        void accept(LightFieldVisitor &visitor) override { }

    private:
        const logical::PanoramicVideoLightField& left_, right_;
        const binaryfunctor &functor_;
    };

    //TODO hacks
    //template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::framerate = 0;
    template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::gop = 0;
    //template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::height = 0;
    //template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::width = 0;
    template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::max_bitrate = 0;
    //template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::rows = 0;
    //template<typename ColorSpace> unsigned int EquirectangularTiledLightField<ColorSpace>::columns = 0;
    template<typename ColorSpace> bool EquirectangularTiledLightField<ColorSpace>::executed = false;
    template<typename ColorSpace> std::shared_ptr<EncodeConfiguration> EquirectangularTiledLightField<ColorSpace>::encodeConfiguration = nullptr;
    template<typename ColorSpace> std::shared_ptr<DecodeConfiguration> EquirectangularTiledLightField<ColorSpace>::decodeConfiguration = nullptr;
    template<typename ColorSpace> std::shared_ptr<GPUContext> EquirectangularTiledLightField<ColorSpace>::context = nullptr;
    template<typename ColorSpace> std::shared_ptr<TileVideoEncoder> EquirectangularTiledLightField<ColorSpace>::tiler = nullptr;
    template<typename ColorSpace> std::string EquirectangularTiledLightField<ColorSpace>::encode_format = "";
    template<typename ColorSpace> std::string EquirectangularTiledLightField<ColorSpace>::decode_format = "";
} // namespace lightdb::physical

#endif //LIGHTDB_OLDPHYSICALOPERATORS_H
