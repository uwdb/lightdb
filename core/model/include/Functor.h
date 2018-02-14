#ifndef LIGHTDB_FUNCTOR_H
#define LIGHTDB_FUNCTOR_H

#include "Color.h"
#include "Geometry.h"
#include "Frame.h"
#include <optional>

namespace lightdb {
    class LightFieldReference;

    //template<typename ColorSpace>
    class functor {
    public:
        virtual ~functor() = default;

        //virtual const typename ColorSpace::Color operator()(const LightField &field,
        //                                                    const Point6D &point) const = 0;
        template<typename ColorSpace=YUVColorSpace>
        const typename ColorSpace::Color operator()(const LightFieldReference &field, const Point6D &point) const {
            return operator()(ColorSpace::Instance, field, point);
        }
        virtual operator const FrameTransform() const { throw BadCastError(); }
        virtual bool hasFrameTransform() const { return false; }

    protected:
        //TODO not happy about doing one heap allocation per interpolation, use a flyweight
        virtual const ColorReference operator()(const ColorSpace&, const LightFieldReference&, const Point6D&) const = 0;
    };

    //TODO use vardaic template to lock number of arguments (e.g., naryfunctor<2>)) and drop vector
    class naryfunctor {
    public:
        virtual ~naryfunctor() = default;

        template<typename ColorSpace>
        const typename ColorSpace::Color operator()(const std::vector<LightFieldReference>& field,
                                                    const Point6D& point) {
            return operator()(ColorSpace::Instance, field, point);
        }
        virtual operator const NaryFrameTransform() const { throw BadCastError(); }
        virtual bool hasFrameTransform() const { return false; }

    protected:
        //TODO not happy about doing one heap allocation per interpolation, use a flyweight
        virtual const ColorReference operator()(const ColorSpace&, const LightFieldReference&, const Point6D&) const = 0;
    };

    //TODO functor namespace
    class Identity: public functor {
    public:
        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const LightFieldReference&, const Point6D&) const override;
    };

    class Greyscale: public functor {
    public:
        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const LightFieldReference&, const Point6D&) const override;
    };

    class GaussianBlur: public functor {
    public:
        GaussianBlur();

        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const LightFieldReference&, const Point6D&) const override;

    private:
        mutable CUmodule module_; //TODO these shouldn't be mutable
        mutable CUfunction function_;
    };

    class DepthmapCPU: public functor {
    public:
        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const LightFieldReference&, const Point6D&) const override {
            throw std::runtime_error("Not implemented");
        }
    };

    class DepthmapGPU: public functor {
    public:
        DepthmapGPU() : module_(nullptr), function_(nullptr) { }

        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const LightFieldReference&, const Point6D&) const override {
            throw std::runtime_error("Not implemented");
        }

    private:
        mutable CUmodule module_; //TODO these shouldn't be mutable
        mutable CUfunction function_;
    };

    class DepthmapFPGA: public functor {
    public:
        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const LightFieldReference&, const Point6D&) const override {
            throw std::runtime_error("Not implemented");
        }
    };

    class Left: public naryfunctor {
    public:
        //operator const NaryFrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const LightFieldReference&, const Point6D&) const override {
            throw std::runtime_error("Not implemented");
        }
    };

    class Overlay: public naryfunctor {
    public:
        explicit Overlay(const YUVColorSpace::Color &transparent)
            : transparent_(transparent), module_(0), function_(0)
        { }

        explicit operator const NaryFrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const LightFieldReference&, const Point6D&) const override {
            throw std::runtime_error("Not implemented");
        }

    private:
        YUVColorSpace::Color transparent_;
        mutable CUmodule module_; //TODO these shouldn't be mutable
        mutable CUfunction function_;
    };
}; // namespace lightdb

#endif //LIGHTDB_FUNCTOR_H
