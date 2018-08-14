#ifndef LIGHTDB_OLDMODEL_H
#define LIGHTDB_OLDMODEL_H

#include "Model.h"

namespace lightdb::logical {
    //TODO These all moved to physical algebra, obvs
    //TODO think now that I've moved geometry to physical algebra, this whole class should die
    class PanoramicLightField : public DiscreteLightField {
    public:
        PanoramicLightField(const Point3D &point,
                            const TemporalRange &time,
                            const ThetaRange &theta = ThetaRange::limits(),
                            const PhiRange &phi = PhiRange::limits())
                : DiscreteLightField(Volume{point, time, theta, phi}/*.ToVolume(range, theta, phi)*/, YUVColorSpace::instance(),
                                     GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples())),
                  point_(point)
        //                   : DiscreteLightField(EquirectangularGeometry::Instance, point.ToVolume(range, theta, phi)),
        //                   point_(point)
        //, volume_{point.ToVolume(range, theta, phi)} {}
        { }
        explicit PanoramicLightField(const TemporalRange &range)
                : PanoramicLightField(Point3D{0, 0, 0}, range) {}

        void accept(LightFieldVisitor &visitor) override { LightField::accept<ConstantLightField>(visitor); }

    protected:
        bool defined_at(const Point6D &point) const override {
            return false; //volume().bounding().contains(point) && EquirectangularGeometry::instance().defined_at(point);
        }

    private:
        const Point3D point_;
    };

    class PanoramicVideoLightField : public PanoramicLightField, public SingletonFileEncodedLightField {
    public:
        explicit PanoramicVideoLightField(const std::string &filename,
                                          const ThetaRange &theta = ThetaRange::limits(),
                                          const PhiRange &phi = PhiRange::limits()) //TODO drop filename; see below
                : PanoramicVideoLightField(filename, Point3D::zero(), theta, phi) {}

        PanoramicVideoLightField(const std::string &filename,
                                 const Point3D &point,
                                 const ThetaRange &theta = ThetaRange::limits(),
                                 const PhiRange &phi = PhiRange::limits()) //TODO drop filename; see below
                : PanoramicLightField(point, {number{0}, number{duration()}}, theta, phi),
                  SingletonFileEncodedLightField(filename,
                                                 (Volume) PanoramicLightField::volume()), //TODO no need to duplicate filename here and in singleton
                  geometry_(Dimension::Time, framerate()) {}

        //const std::vector<LightFieldReference> parents() const override { return {}; }

        //const ColorSpace colorSpace() const override { return YUVColorSpace::Instance; } //TODO pull from file
        //TODO remove both of these
        inline number framerate() const { return {rational(30, 1)}; } //TODO hardcoded...

        inline size_t duration() const { return 99; } //TODO remove hardcoded value
        inline utility::StreamMetadata metadata() const {
            return (metadata_.has_value()
                    ? metadata_
                    : (metadata_ = utility::StreamMetadata(filename(), 0, true))).value();
        }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<ConstantLightField>(visitor); }

    protected:
        bool defined_at(const Point6D &point) const override {
            return geometry_.defined_at(point) &&
                   PanoramicLightField::defined_at(point);
        }

    private:
        //TODO drop filename after adding StreamDecodeReader in Physical.cc
        // Can make SingletonEncodedLightField be a replacement for this class; one for in-memory, one for on-disk
        const IntervalGeometry geometry_;
        mutable std::optional<utility::StreamMetadata> metadata_;
    };

    class PlanarTiledVideoLightField : public DiscreteLightField, public SingletonFileEncodedLightField {
    public:
        PlanarTiledVideoLightField(const std::string &filename,
                                   const Volume &volume,
                                   const size_t rows, const size_t columns)
                : DiscreteLightField(volume, YUVColorSpace::instance(),
                                     GeometryReference::make<IntervalGeometry>(Dimension::Time, framerate())),
                  SingletonFileEncodedLightField(filename,
                                                 volume), //TODO no need to duplicate filename here and in singleton
                //volume_(volume),
                  rows_(rows), columns_(columns) {}

        //const std::vector<LightFieldReference> parents() const override { return {}; }

        //const ColorSpace colorSpace() const override { return YUVColorSpace::Instance; } //TODO pull from file
        //TODO remove both of these
        inline rational framerate() const { return rational(30, 1); } //TODO hardcoded...

        inline size_t duration() const { return 20; } //TODO remove hardcoded value
        inline utility::StreamMetadata metadata() const {
            return (metadata_.has_value()
                    ? metadata_
                    : (metadata_ = utility::StreamMetadata(filename(), 0, true))).value();
        }

        void accept(LightFieldVisitor &visitor) override { LightField::accept<ConstantLightField>(visitor); }

    private:
        //TODO drop filename after adding StreamDecodeReader in Physical.cc
        // Can make SingletonEncodedLightField be a replacement for this class; one for in-memory, one for on-disk, one for streaming
        //const Volume volume_;
        const size_t rows_, columns_;
        mutable std::optional<utility::StreamMetadata> metadata_;
    };
};
#endif //LIGHTDB_OLDMODEL_H
