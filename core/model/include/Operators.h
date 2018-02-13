#ifndef LIGHTDB_OPERATORS_H
#define LIGHTDB_OPERATORS_H

#include "LightField.h"
#include "Encoding.h"
#include "Execution.h"
#include "Interpolation.h"
#include "Ffmpeg.h"
#include "rational.h"
#include <memory>
#include <functional>
#include <fstream>
#include <iterator>

namespace lightdb {

    class Operator {

    };

    class UnaryOperator : public Operator {
    public:
        virtual LightFieldReference apply(const LightFieldReference&) const = 0;
    };

    class BinaryOperator : public Operator {
    public:
        virtual LightFieldReference apply(const LightFieldReference, const LightFieldReference) const = 0;
    };


    class Encode : public Operator {
    public:
        //TODO string as a format is horrible, fix
        Encode()
                : format_("hevc") {}

        explicit Encode(std::string &&format)
                : format_(format) {}

        EncodedLightField apply(const LightFieldReference &lightField) const { //TODO ostream
            return lightdb::pipeline::execute(lightField, format_);
        }

    private:
        const std::string format_;
    };


    class Decode : public Operator {
    public:
        //TODO constructors should accept EncodedLightFields, not string/streams
        //TODO theta and phi should be drawn from the container, not explicitly parameterized
        Decode(const std::string &filename,
               const AngularRange &theta = AngularRange::ThetaMax,
               const AngularRange &phi = AngularRange::PhiMax)
        //: Decode(std::ifstream{filename})
                : field_(std::shared_ptr<LightField>(
                new PanoramicVideoLightField(filename, theta, phi))) {}

        Decode(const bool forceLightField,
               const std::string &filename,
               const AngularRange &theta = AngularRange::ThetaMax,
               const AngularRange &phi = AngularRange::PhiMax)
        //: Decode(std::ifstream{filename})
                : field_(std::shared_ptr<LightField>(
                      new PlanarTiledVideoLightField(filename,
                                                    Volume{{0,
                                                            1},
                                                           {0,
                                                            1},
                                                           {0,
                                                            0},
                                                           TemporalRange::TemporalMax,
                                                           theta,
                                                           phi},
                                                    3, 3))) {}

        //TODO Decode(std::istream &&stream)
               // : field_(std::shared_ptr<LightField>(
                //new PanoramicVideoLightField(std::move(stream)))) {}

        operator const LightFieldReference() const {
            return field_;
        }

        LightFieldReference apply() const {
            return field_;
        }

        const LightFieldReference operator>>(const UnaryOperator &op) const {
            return op.apply(field_);
            //return field_ >> op;
        }

        const EncodedLightField operator>>(const Encode &op) const {
            return op.apply(field_);
            //return field_ >> op;
        }

    private:
        const LightFieldReference field_;
    };


    template<typename ColorSpace>
    class Scan : public Operator {
        LightField &&scan(std::string name) {
            return ConstantLightField(YUVColor::Green); //TODO
        }
    };

    class Store : public Operator {
    public:
        explicit Store(const std::string &name)
                : name_(name) {}

        lightdb::EncodedLightField apply(const EncodedLightField &encoded) const {
            encoded->write(name_);
            return encoded;
        }

        //TODO can also store an unencoded query by just default-encoding it; add new >> and apply overloads

    private:
        const std::string name_;
    };

//template<typename LeftColorSpace, typename RightColorSpace, typename OutColorSpace>
    class Union : public BinaryOperator { //TODO
//class Union: public BinaryOperator<LeftColorSpace, RightColorSpace, OutColorSpace> {
    public:
        enum MergeType {
            Left,
            Right
        };

        Union()
                : Union(MergeType::Left) {}

        explicit Union(const MergeType mergeType)
                : mergeType_(mergeType) {}

        LightFieldReference apply(const LightFieldReference left, const LightFieldReference right) const {
            return LightFieldReference::make<CompositeLightField>(
                    std::vector<LightFieldReference>{left, right});
        }

    private:
        const MergeType mergeType_;
    };

    class Select : public UnaryOperator { //TODO
    public:
        explicit Select(const Volume &volume)
                : volume_(volume) {}

        LightFieldReference apply(const LightFieldReference &field) const override {
            return LightFieldReference::make<SubsetLightField>(field, volume_);
        }

    private:
        const Volume volume_;
    };

    class Rotate : public UnaryOperator { //TODO
    public:
        Rotate(const angle theta, const angle phi)
                : theta_(theta), phi_(phi) {}

        LightFieldReference apply(const LightFieldReference &field) const override {
            return LightFieldReference::make<RotatedLightField>(field, theta_, phi_);
        }

    private:
        const angle theta_, phi_;
    };

    class Partition : public UnaryOperator { //TODO
    public:
        Partition(const Dimension &dimension, const rational interval)
            : dimension_(dimension), interval_(interval) { }

        Dimension dimension() const { return dimension_; }

        lightdb::rational interval() const { return interval_; }

        LightFieldReference apply(const LightFieldReference &field) const override {
            return LightFieldReference::make<PartitionedLightField>(
                    field, dimension(), interval());
//                PartitionedLightField<YUVColorSpace>(field, dimension(), delta());
        }

    private:
        const Dimension dimension_;
        const lightdb::rational interval_;
    };


    using bitrate = unsigned int;

    class Transcode : public UnaryOperator { //TODO
    public:
        explicit Transcode(const std::function<bitrate(Volume &)> &bitrater)
            : bitrater_(bitrater) {}

        LightFieldReference apply(const LightFieldReference &field) const override {
            //TODO clean this up, should be able to decode from memory
            auto encoded = Encode("hevc").apply(field);

            encoded->write("out*");

            std::vector<LightFieldReference> decodes;
            for (auto i = 0u; i < encoded->segments().size(); i++) {
                auto filename = std::string("out") + std::to_string(i); //+ ".hevc";

                decodes.emplace_back(Decode(filename, encoded->volume().components()[i].theta, encoded->volume().components()[i].phi).apply());
            }

            return LightFieldReference::make<CompositeLightField>(decodes);
        }

    private:
        const std::function<bitrate(Volume &)> bitrater_;
    };

    class Interpolate : public UnaryOperator { //TODO
    public:
        explicit Interpolate(const Dimension dimension, const interpolation::interpolator<YUVColorSpace> &interpolator)
                : dimension_(dimension), interpolator_(interpolator) {}

        LightFieldReference apply(const LightFieldReference &field) const override {
            return LightFieldReference::make<InterpolatedLightField>(
                    field, dimension_, interpolator_);
        }

    private:
        const Dimension dimension_;
        const interpolation::interpolator<YUVColorSpace> &interpolator_;
    };

    class Discretize : public UnaryOperator { //TODO
    public:
        Discretize(const Dimension &dimension, lightdb::rational &&interval)
                : Discretize(IntervalGeometry(dimension, interval)) {}

        Discretize(const Dimension &dimension, lightdb::rational &interval)
                : Discretize(IntervalGeometry(dimension, interval)) {}

        Discretize(const Geometry &&geometry)
                : geometry_(geometry) {}

        LightFieldReference apply(const LightFieldReference &field) const override {
            return LightFieldReference::make<DiscretizedLightField>(field, geometry_);
        }

    private:
        const Geometry &geometry_;
    };

    class Map : public UnaryOperator { //TODO
    public:
        Map(lightdb::functor<YUVColorSpace> &functor)
                : functor_(functor) {}

        Map(lightdb::functor<YUVColorSpace> &&functor)
                : functor_(functor) {}

        LightFieldReference apply(const LightFieldReference &field) const override {
            return LightFieldReference::make<TransformedLightField>(field, functor_);
        }

    private:
        const lightdb::functor<YUVColorSpace> &functor_;
    };

//    template<typename InColorSpace, typename OutColorSpace>
    inline LightFieldReference operator>>(const LightFieldReference &input,
                                          const UnaryOperator &op) {
        return op.apply(input);
    }

//   template<typename InColorSpace, typename OutColorSpace>
    inline LightFieldReference operator>>(LightField &input,
                                          const UnaryOperator &op) {
        throw std::runtime_error("Used to be implemented, but then removed with color spaces");
        //return op.apply(LightFieldReference(input));
    }

//    template<typename ColorSpace>
    inline lightdb::EncodedLightField operator>>(const LightFieldReference &input,
                                                 const Encode &encoder) {
        return encoder.apply(LightFieldReference(input));
    }

   // template<typename ColorSpace>
    inline lightdb::EncodedLightField operator>>(const LightFieldReference &&input,
                                                 const Encode &encoder) {
        return encoder.apply(LightFieldReference(input));
    }

//    template<typename ColorSpace>
    inline LightFieldReference operator|(const LightFieldReference &left,
                                         const LightFieldReference &right) {
        return Union().apply(left, right);
    }

    inline lightdb::EncodedLightField operator>>(const lightdb::EncodedLightField &input, const Store &store) {
        return store.apply(input);
    }

    inline lightdb::EncodedLightField operator>>(lightdb::EncodedLightField &&input, const Store &store) {
        return store.apply(input);
    }

} // namespace lightdb

#endif //LIGHTDB_OPERATORS_H
