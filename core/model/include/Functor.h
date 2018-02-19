#ifndef LIGHTDB_FUNCTOR_H
#define LIGHTDB_FUNCTOR_H

#include "Color.h"
#include "Geometry.h"
#include "Frame.h"
#include <optional>

namespace lightdb {
    class LightFieldReference;

    namespace internal {
        template<typename, typename>
        class _nary_builder;

        template<typename T, std::size_t... S>
        class _nary_builder<T, std::index_sequence<S...>> {
            template<std::size_t>
            using type = T;

            //TODO not happy about doing one heap allocation per interpolation, use a flyweight
            virtual const ColorReference operator()(const ColorSpace &, const Point6D &, type<S>...) const = 0;
        };
    } // namespace internal

    //TODO functor namespace?
    template<std::size_t n>
    class naryfunctor: internal::_nary_builder<const LightFieldReference&, std::make_index_sequence<n>> {
    public:
        virtual ~naryfunctor() = default;

        // Clean this up, unify FrameTransform and NaryFrameTransform
        virtual explicit operator const FrameTransform() const { throw BadCastError(); }
        virtual operator const NaryFrameTransform() const { throw BadCastError(); }
        virtual bool hasFrameTransform() const { return false; }
    };

    using unaryfunctor = naryfunctor<1>;
    using binaryfunctor = naryfunctor<2>;
    using FunctorReference = shared_reference<unaryfunctor>;

    class Identity: public unaryfunctor {
    public:
        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const Point6D&, const LightFieldReference&) const override;
    };

    class Greyscale: public unaryfunctor {
    public:
        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const Point6D&, const LightFieldReference&) const override;
    };

    class GaussianBlur: public unaryfunctor {
    public:
        GaussianBlur();

        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const Point6D&, const LightFieldReference&) const override;

    private:
        mutable CUmodule module_; //TODO these shouldn't be mutable
        mutable CUfunction function_;
    };

    class DepthmapCPU: public unaryfunctor {
    public:
        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const Point6D&, const LightFieldReference&) const override {
            throw std::runtime_error("Not implemented");
        }
    };

    class DepthmapGPU: public unaryfunctor {
    public:
        DepthmapGPU() : module_(nullptr), function_(nullptr) { }

        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const Point6D&, const LightFieldReference&) const override {
            throw std::runtime_error("Not implemented");
        }

    private:
        mutable CUmodule module_; //TODO these shouldn't be mutable
        mutable CUfunction function_;
    };

    class DepthmapFPGA: public unaryfunctor {
    public:
        explicit operator const FrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const Point6D&, const LightFieldReference&) const override {
            throw std::runtime_error("Not implemented");
        }
    };

    class Left: public naryfunctor<2> {
    public:
        //operator const NaryFrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const Point6D&, const LightFieldReference&,
                                        const LightFieldReference&) const override {
            throw std::runtime_error("Not implemented");
        }
    };

    class Overlay: public naryfunctor<2> {
    public:
        explicit Overlay(const YUVColorSpace::Color &transparent)
            : transparent_(transparent), module_(0), function_(0)
        { }

        explicit operator const NaryFrameTransform() const override;
        bool hasFrameTransform() const override { return true; }

    protected:
        const ColorReference operator()(const ColorSpace&, const Point6D&, const LightFieldReference&, const LightFieldReference&) const override {
            throw std::runtime_error("Not implemented");
        }

    private:
        YUVColorSpace::Color transparent_;
        mutable CUmodule module_; //TODO these shouldn't be mutable
        mutable CUfunction function_;
    };
}; // namespace lightdb

#endif //LIGHTDB_FUNCTOR_H
