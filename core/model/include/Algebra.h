#ifndef LIGHTDB_ALGEBRA_H
#define LIGHTDB_ALGEBRA_H

#include "LightField.h"
#include "Catalog.h"
#include "Geometry.h"
#include "Interpolation.h"
#include "options.h"

namespace lightdb {
    namespace functor {
        template<std::size_t n> class naryfunctor;
        template<std::size_t n> using FunctorReference = shared_reference<naryfunctor<n>>;
        using UnaryFunctorReference = FunctorReference<1>;
    }

    namespace logical {
        LightFieldReference Scan(const std::string &name);
        LightFieldReference Scan(const catalog::Catalog&, const std::string &name);

        class Algebra: public DefaultMixin {
        public:
            //TODO Decode, Transcode

            //LightFieldReference Load(const std::string &uri);
            LightFieldReference Select(const Volume&);
            LightFieldReference Select(SpatiotemporalDimension, const SpatiotemporalRange&);
            LightFieldReference Select(const ThetaRange&);
            LightFieldReference Select(const PhiRange&);
            LightFieldReference Union(std::vector<LightFieldReference>); //TODO needs merge function
            LightFieldReference Union(LightFieldReference); //TODO needs merge function
            LightFieldReference Rotate(angle theta, angle phi);
            LightFieldReference Partition(Dimension, const number&);
            LightFieldReference Interpolate(Dimension, interpolation::InterpolatorReference);
            LightFieldReference Discretize(const GeometryReference&);
            LightFieldReference Discretize(Dimension, const number&);
            LightFieldReference Map(functor::UnaryFunctorReference);
            LightFieldReference Subquery(const std::function<LightFieldReference(LightFieldReference)>&);

            LightFieldReference Encode(const Codec& = Codec::hevc(), const lightdb::options<>& = {});

            LightFieldReference Store(const std::string &name, const Codec &codec=Codec::hevc());
            LightFieldReference Sink();

            Algebra& operator=(Algebra&& other) noexcept { return *this; }

        protected:
            explicit Algebra(const LightFieldReference &lightField)
                    : DefaultMixin(lightField), this_(lightField)
            { }

        private:
            const LightFieldReference &this_;
        };
    } // namespace logical
} // namespace lightdb

#endif //LIGHTDB_ALGEBRA_H
