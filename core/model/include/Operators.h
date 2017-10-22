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


template<typename Geometry, typename ColorSpace=YUVColorSpace>
class Decode: public Operator {
public:
    //TODO constructors should accept EncodedLightFields, not string/streams

    Decode(const std::string &filename)
        : Decode(std::ifstream{filename})
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

    const LightFieldReference<ColorSpace> operator>>(const UnaryOperator<ColorSpace, ColorSpace>& op) {
        return field_ >> op;
    }

private:
    const LightFieldReference<ColorSpace> field_;
};


template<typename ColorSpace>
class Encode: public Operator {
public:
    visualcloud::EncodedLightField apply(const LightFieldReference<ColorSpace> &lightField) const { //TODO ostream
        return visualcloud::pipeline::execute(lightField);
    }
};

template<typename ColorSpace>
class Scan: public Operator {
    LightField<ColorSpace>&& scan(std::string name) {
        return ConstantLightField<YUVColorSpace>(YUVColor::Green); //TODO
    }
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
        printf("Transcode.apply");
        return ConstantLightField<YUVColorSpace>::create(YUVColor::Green);
    }

private:
    const std::function<bitrate(Volume&)> bitrater_;
};

class Interpolate: public UnaryOperator<YUVColorSpace, YUVColorSpace> { //TODO
public:
    explicit Interpolate(const visualcloud::interpolator<YUVColorSpace> &interpolator)
            : interpolator_(interpolator)
    { }

    LightFieldReference<YUVColorSpace> apply(const LightFieldReference<YUVColorSpace>& field) const override {
        return LightFieldReference<YUVColorSpace>::make<InterpolatedLightField<YUVColorSpace>>(field, interpolator_);
    }

private:
    const visualcloud::interpolator<YUVColorSpace> &interpolator_;
};

class Discretize: public UnaryOperator<YUVColorSpace, YUVColorSpace> { //TODO
public:
    explicit Discretize(const Dimension &dimension, visualcloud::rational &&interval)
        : dimension_(dimension), interval_(interval)
    { }

    explicit Discretize(const Dimension &dimension, visualcloud::rational &interval)
        : dimension_(dimension), interval_(interval)
    { }

    LightFieldReference<YUVColorSpace> apply(const LightFieldReference<YUVColorSpace>& field) const override {
        return LightFieldReference<YUVColorSpace>::make<DiscretizedLightField<YUVColorSpace>>(
                field, IntervalGeometry(dimension_, interval_));
    }

private:
    const Dimension dimension_;
    const visualcloud::rational interval_;
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
inline visualcloud::EncodedLightField operator>>(LightFieldReference<ColorSpace>& input,
                                                 const Encode<ColorSpace>& encoder)
{
    return encoder.apply(LightFieldReference<ColorSpace>(input));
}

template<typename ColorSpace>
inline visualcloud::EncodedLightField operator>>(LightFieldReference<ColorSpace>&& input,
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

#endif //VISUALCLOUD_OPERATORS_H
