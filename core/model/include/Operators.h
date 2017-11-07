#ifndef VISUALCLOUD_OPERATORS_H
#define VISUALCLOUD_OPERATORS_H

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

//TODO namespace visualcloud {

class Operator {

};

template<typename InColorSpace, typename OutColorSpace>
class UnaryOperator: public Operator {
public:
    virtual LightFieldReference<OutColorSpace> apply(const LightFieldReference<InColorSpace>&) const = 0;
};

template<typename LeftColorSpace, typename RightColorSpace, typename OutColorSpace>
class BinaryOperator: public Operator {
public:
    virtual LightFieldReference<OutColorSpace> apply(const LightFieldReference<LeftColorSpace>,
                                                     const LightFieldReference<RightColorSpace>) const = 0;
};


template<typename ColorSpace>
class Encode: public Operator {
public:
    //TODO string as a format is horrible, fix
    Encode()
            : format_("hevc")
    { }

    Encode(std::string &&format)
            : format_(format)
    { }

    visualcloud::EncodedLightField apply(const LightFieldReference<ColorSpace> &lightField) const { //TODO ostream
        return visualcloud::pipeline::execute(lightField, format_);
    }

private:
    const std::string format_;
};


template<typename Geometry, typename ColorSpace=YUVColorSpace>
class Decode: public Operator {
public:
    //TODO constructors should accept EncodedLightFields, not string/streams
    //TODO theta and phi should be drawn from the container, not explicitly parameterized
    Decode(const std::string &filename,
           const AngularRange& theta=AngularRange::ThetaMax,
           const AngularRange& phi=AngularRange::PhiMax)
        //: Decode(std::ifstream{filename})
            : field_(std::shared_ptr<LightField<ColorSpace>>(new PanoramicVideoLightField<Geometry, ColorSpace>(filename, theta, phi)))

    { }

    Decode(std::istream &&stream)
         : field_(std::shared_ptr<LightField<ColorSpace>>(
                      new PanoramicVideoLightField<Geometry, ColorSpace>(std::move(stream))))
    { }

    operator const LightFieldReference<ColorSpace>() const {
        return field_;
    }

    LightFieldReference<ColorSpace> apply() const {
        return field_;
    }

    const LightFieldReference<ColorSpace> operator>>(const UnaryOperator<ColorSpace, ColorSpace>& op) const {
        return field_ >> op;
    }

    const visualcloud::EncodedLightField operator>>(const Encode<ColorSpace>& op) const {
        return field_ >> op;
    }

private:
    const LightFieldReference<ColorSpace> field_;
};


template<typename ColorSpace>
class Scan: public Operator {
    LightField<ColorSpace>&& scan(std::string name) {
        return ConstantLightField<YUVColorSpace>(YUVColor::Green); //TODO
    }
};

class Store: public Operator {
public:
    Store(const std::string &name)
        : name_(name)
    { }

    visualcloud::EncodedLightField apply(const visualcloud::EncodedLightField &encoded) const {
        encoded->write(name_);
        return encoded;
    }

    //TODO can also store an unencoded query by just default-encoding it; add new >> and apply overloads

private:
    const std::string name_;
};

//template<typename LeftColorSpace, typename RightColorSpace, typename OutColorSpace>
class Union: public BinaryOperator<YUVColorSpace, YUVColorSpace, YUVColorSpace> { //TODO
//class Union: public BinaryOperator<LeftColorSpace, RightColorSpace, OutColorSpace> {
public:
    enum MergeType {
        Left,
        Right
    };

    Union()
        : Union(MergeType::Left)
    { }

    Union(const MergeType mergeType)
        : mergeType_(mergeType)
    { }

    LightFieldReference<YUVColorSpace> apply(const LightFieldReference<YUVColorSpace> left,
                                             const LightFieldReference<YUVColorSpace> right) const {
        return LightFieldReference<YUVColorSpace>::make<CompositeLightField<YUVColorSpace>>(
                std::vector<LightFieldReference<YUVColorSpace>>{left, right});
    }

private:
    const MergeType mergeType_;
};

class Select: public UnaryOperator<YUVColorSpace, YUVColorSpace> { //TODO
public:
    explicit Select(const Volume &volume)
            : volume_(volume)
    { }

    LightFieldReference<YUVColorSpace> apply(const LightFieldReference<YUVColorSpace>& field) const override {
        return LightFieldReference<YUVColorSpace>::make<SubsetLightField<YUVColorSpace>>(field, volume_);
    }

private:
    const Volume volume_;
};

class Rotate: public UnaryOperator<YUVColorSpace, YUVColorSpace> { //TODO
public:
    explicit Rotate(const angle theta, const angle phi)
            : theta_(theta), phi_(phi)
    { }

    LightFieldReference<YUVColorSpace> apply(const LightFieldReference<YUVColorSpace>& field) const override {
        return LightFieldReference<YUVColorSpace>::make<RotatedLightField<YUVColorSpace>>(field, theta_, phi_);
    }

private:
    const angle theta_, phi_;
};

class Partition: public UnaryOperator<YUVColorSpace, YUVColorSpace> { //TODO
public:
    Partition(const Dimension &dimension, const visualcloud::rational interval)
        : dimension_(dimension), interval_(interval)
    { }

    Dimension dimension() const { return dimension_; }
    visualcloud::rational interval() const { return interval_; }

    LightFieldReference<YUVColorSpace> apply(const LightFieldReference<YUVColorSpace>& field) const override {
        return LightFieldReference<YUVColorSpace>::make<PartitionedLightField<YUVColorSpace>>(
                field, dimension(), interval());
//                PartitionedLightField<YUVColorSpace>(field, dimension(), delta());
    }

private:
    const Dimension dimension_;
    const visualcloud::rational interval_;
};


using bitrate = unsigned int;

class Transcode: public UnaryOperator<YUVColorSpace, YUVColorSpace> { //TODO
public:
    explicit Transcode(const std::function<bitrate(Volume&)> &bitrater)
        : bitrater_(bitrater)
    { }

    LightFieldReference<YUVColorSpace> apply(const LightFieldReference<YUVColorSpace>& field) const override {
        //TODO clean this up, should be able to decode from memory
        auto encoded = Encode<YUVColorSpace>("hevc").apply(field);

        encoded->write("out*");

        std::vector<LightFieldReference<YUVColorSpace>> decodes;
        for(auto i = 0u; i < encoded->segments().size(); i++) {
            auto filename = std::string("out") + std::to_string(i); //+ ".hevc";

            decodes.emplace_back(Decode<EquirectangularGeometry>(filename, encoded->volumes()[i].theta, encoded->volumes()[i].phi).apply());
        }

        return LightFieldReference<YUVColorSpace>::make<CompositeLightField<YUVColorSpace>>(decodes);
    }

private:
    const std::function<bitrate(Volume&)> bitrater_;
};

class Interpolate: public UnaryOperator<YUVColorSpace, YUVColorSpace> { //TODO
public:
    explicit Interpolate(const Dimension dimension, const visualcloud::interpolator<YUVColorSpace> &interpolator)
            : dimension_(dimension), interpolator_(interpolator)
    { }

    LightFieldReference<YUVColorSpace> apply(const LightFieldReference<YUVColorSpace>& field) const override {
        return LightFieldReference<YUVColorSpace>::make<InterpolatedLightField<YUVColorSpace>>(
                field, dimension_, interpolator_);
    }

private:
    const Dimension dimension_;
    const visualcloud::interpolator<YUVColorSpace> &interpolator_;
};

class Discretize: public UnaryOperator<YUVColorSpace, YUVColorSpace> { //TODO
public:
    Discretize(const Dimension &dimension, visualcloud::rational &&interval)
        : Discretize(IntervalGeometry(dimension, interval))
    { }

    Discretize(const Dimension &dimension, visualcloud::rational &interval)
        : Discretize(IntervalGeometry(dimension, interval))
    { }

    Discretize(const Geometry &&geometry)
            : geometry_(geometry)
    { }

    LightFieldReference<YUVColorSpace> apply(const LightFieldReference<YUVColorSpace>& field) const override {
        return LightFieldReference<YUVColorSpace>::make<DiscretizedLightField<YUVColorSpace>>(field, geometry_);
    }

private:
    const Geometry &geometry_;
};

class Map: public UnaryOperator<YUVColorSpace, YUVColorSpace> { //TODO
public:
    Map(visualcloud::functor<YUVColorSpace> &functor)
        : functor_(functor)
    { }

    Map(visualcloud::functor<YUVColorSpace> &&functor)
            : functor_(functor)
    { }

    LightFieldReference<YUVColorSpace> apply(const LightFieldReference<YUVColorSpace>& field) const override {
        return LightFieldReference<YUVColorSpace>::make<TransformedLightField<YUVColorSpace>>(field, functor_);
    }

private:
    const visualcloud::functor<YUVColorSpace> &functor_;
};

template<typename InColorSpace, typename OutColorSpace>
inline LightFieldReference<OutColorSpace> operator>>(const LightFieldReference<InColorSpace>& input,
                                                     const UnaryOperator<InColorSpace, OutColorSpace>& op)
{
    return op.apply(input);
}

template<typename InColorSpace, typename OutColorSpace>
inline LightFieldReference<OutColorSpace> operator>>(LightField<InColorSpace>& input,
                                                    const UnaryOperator<InColorSpace, OutColorSpace>& op)
{
    return op.apply(LightFieldReference<InColorSpace>(input));
}

template<typename ColorSpace>
inline visualcloud::EncodedLightField operator>>(const LightFieldReference<ColorSpace>& input,
                                                 const Encode<ColorSpace>& encoder)
{
    return encoder.apply(LightFieldReference<ColorSpace>(input));
}

template<typename ColorSpace>
inline visualcloud::EncodedLightField operator>>(const LightFieldReference<ColorSpace>&& input,
                                                 const Encode<ColorSpace>& encoder)
{
    return encoder.apply(LightFieldReference<ColorSpace>(input));
}

template<typename ColorSpace>
inline LightFieldReference<ColorSpace> operator|(const LightFieldReference<ColorSpace>& left,
                                                 const LightFieldReference<ColorSpace>& right)
{
    return Union().apply(left, right);
}

inline visualcloud::EncodedLightField operator>>(const visualcloud::EncodedLightField& input, const Store& store)
{
    return store.apply(input);
}

inline visualcloud::EncodedLightField operator>>(visualcloud::EncodedLightField&& input, const Store& store)
{
    return store.apply(input);
}

#endif //VISUALCLOUD_OPERATORS_H
