#ifndef LIGHTDB_ALGEBRA_H
#define LIGHTDB_ALGEBRA_H

#include "Catalog.h"
#include "Geometry.h"
#include "Functor.h"
#include "Interpolation.h"

namespace lightdb {
    class LightFieldReference;

    namespace logical {
        LightFieldReference Scan(const std::string &name);
        LightFieldReference Scan(const catalog::Catalog&, const std::string &name);

        class Algebra {
        public:
            //TODO Encode, Decode, Transcode

            //LightFieldReference Load(const std::string &uri);
            LightFieldReference Select(const Volume&);
            LightFieldReference Select(SpatiotemporalDimension, const SpatiotemporalRange&);
            LightFieldReference Select(const ThetaRange&);
            LightFieldReference Select(const PhiRange&);
            LightFieldReference Union(const std::vector<LightFieldReference>&); //TODO needs merge function
            LightFieldReference Union(LightFieldReference); //TODO needs merge function
            LightFieldReference Rotate(angle theta, angle phi);
            LightFieldReference Partition(Dimension, number);
            LightFieldReference Interpolate(Dimension, const interpolation::interpolator&);
            LightFieldReference Discretize(const GeometryReference&);
            LightFieldReference Discretize(Dimension, number);
            LightFieldReference Map(FunctorReference);
            void Store(const std::string &name);

        protected:
            explicit Algebra(LightFieldReference &lightField) : this_(lightField) { }

        private:
            const LightFieldReference &this_;
        };
    } // namespace logical
} // namespace lightdb

#endif //LIGHTDB_ALGEBRA_H
