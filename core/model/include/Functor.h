#ifndef LIGHTDB_FUNCTOR_H
#define LIGHTDB_FUNCTOR_H

#include "Color.h"
#include "Frame.h"
#include <optional>

namespace lightdb {
    template<typename ColorSpace>
    class functor {
    public:
        virtual ~functor() { }

        virtual const typename ColorSpace::Color operator()(const LightField<ColorSpace> &field,
                                                            const Point6D &point) const = 0;
        virtual const typename ColorSpace::Color operator()(const LightFieldReference<ColorSpace> &field,
                                                            const Point6D &point) const {
            return this->operator()(*field, point);
        }
        virtual operator const FrameTransform() const { throw std::bad_cast(); }
        virtual bool hasFrameTransform() const { return false; }
    };

    template<typename ColorSpace>
    class naryfunctor {
    public:
        virtual ~naryfunctor() { }

        virtual const typename ColorSpace::Color operator()(const std::vector<LightFieldReference<ColorSpace>> &fields,
                                                            const Point6D &point) = 0;
        virtual operator const NaryFrameTransform() const { throw std::bad_cast(); }
        virtual bool hasFrameTransform() const { return false; }
    };

    //TODO functor namespace
    class Identity: public functor<YUVColorSpace> {
    public:
        ~Identity() { }

        const YUVColorSpace::Color operator()(const LightField<YUVColorSpace> &field,
                                              const Point6D &point) const override;
        operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }
    };

    class Greyscale: public functor<YUVColorSpace> {
    public:
        ~Greyscale() { }

        const YUVColorSpace::Color operator()(const LightField<YUVColorSpace> &field,
                                              const Point6D &point) const override;

        operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }
    };

    class GaussianBlur: public functor<YUVColorSpace> {
    public:
        GaussianBlur();
        ~GaussianBlur() { }

        const YUVColorSpace::Color operator()(const LightField<YUVColorSpace> &field,
                                              const Point6D &point) const override;

        operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    private:
        mutable CUmodule module_; //TODO these shouldn't be mutable
        mutable CUfunction function_;
    };

    class DepthmapCPU: public functor<YUVColorSpace> {
    public:
        ~DepthmapCPU() { }

        const YUVColorSpace::Color operator()(const LightField<YUVColorSpace> &field,
                                              const Point6D &point) const override {
            throw new std::runtime_error("Not implemented");
        }

        operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }
    };

    class DepthmapGPU: public functor<YUVColorSpace> {
    public:
        DepthmapGPU() : module_(nullptr), function_(nullptr) { }
        ~DepthmapGPU() { }

        const YUVColorSpace::Color operator()(const LightField<YUVColorSpace> &field,
                                              const Point6D &point) const override {
            throw new std::runtime_error("Not implemented");
        }

        operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    private:
        mutable CUmodule module_; //TODO these shouldn't be mutable
        mutable CUfunction function_;
    };

    class DepthmapFPGA: public functor<YUVColorSpace> {
    public:
        ~DepthmapFPGA() { }

        const YUVColorSpace::Color operator()(const LightField<YUVColorSpace> &field,
                                              const Point6D &point) const override {
            throw new std::runtime_error("Not implemented");
        }

        operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }
    };

    class Left: public naryfunctor<YUVColorSpace> {
    public:
        ~Left() { }

        const YUVColorSpace::Color operator()(const std::vector<LightFieldReference<YUVColorSpace>> &fields,
                                              const Point6D &point) override {
            throw std::runtime_error("Not implemented");
        }

        operator const NaryFrameTransform() const override;
        bool hasFrameTransform() const override { return true; }
    };

    class Overlay: public naryfunctor<YUVColorSpace> {
    public:
        Overlay(YUVColorSpace::Color transparent) : transparent_(transparent) { }
        ~Overlay() { }

        const YUVColorSpace::Color operator()(const std::vector<LightFieldReference<YUVColorSpace>> &fields,
                                              const Point6D &point) override {
            throw std::runtime_error("Not implemented");
        }

        operator const NaryFrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    private:
        YUVColorSpace::Color transparent_;
        mutable CUmodule module_; //TODO these shouldn't be mutable
        mutable CUfunction function_;
    };
}; // namespace lightdb

#endif //LIGHTDB_FUNCTOR_H
