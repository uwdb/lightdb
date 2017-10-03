#ifndef VISUALCLOUD_OPERATORS_H
#define VISUALCLOUD_OPERATORS_H

#include "LightField.h"
#include <libavcodec/avcodec.h>
#include <memory>

class Operator {

};

template<typename InColorSpace, typename OutColorSpace>
class UnaryOperator: public Operator {
public:
    LightField<OutColorSpace> apply(LightField<InColorSpace>&) const = 0;
};

template<typename LeftColorSpace, typename RightColorSpace, typename OutColorSpace>
class BinaryOperator: public Operator {
public:
    virtual std::shared_ptr<LightField<OutColorSpace>> apply(LightField<LeftColorSpace>&, LightField<RightColorSpace>&) const = 0;
};


template<typename ColorSpace>
class Decode: public Operator {
public:
    ConstantLightField<ColorSpace> apply(std::istream &stream) {
        return ConstantLightField<YUVColorSpace>(YUVColor::Green); //TODO
    }

    ConstantLightField<ColorSpace> apply(std::istream &&stream) {
        return ConstantLightField<YUVColorSpace>(YUVColor::Green); //TODO
    }
};

template<typename ColorSpace>
class Encode: public Operator {
public:
    void* apply(LightField<ColorSpace> &lightField) { //TODO ostream
        return nullptr; //TODO
    }

    void* apply(std::shared_ptr<LightField<ColorSpace>> lightField) { //TODO ostream
        return nullptr; //TODO
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
    Union() {
    }

    std::shared_ptr<LightField<YUVColorSpace>> apply(LightField<YUVColorSpace>& left, LightField<YUVColorSpace>& right) const override {
        return std::make_shared<ConstantLightField<YUVColorSpace>>(YUVColor::Green); //TODO
    }

    //std::shared_ptr<LightField<YUVColorSpace>> apply() {
    //    return std::make_shared<ConstantLightField<YUVColorSpace>>(YUVColor::Green); //TODO
    //}

};

#endif //VISUALCLOUD_OPERATORS_H
