#ifndef LIGHTDB_ALGEBRA_H
#define LIGHTDB_ALGEBRA_H

#include "Geometry.h"
#include "Functor.h"
#include "Interpolation.h"

namespace lightdb {
    class LightFieldReference;

    namespace logical {

        class Algebra {
        public:
            //TODO Scan, Encode, Decode, Transcode

            void Store(const std::string &name);
            LightFieldReference Select(const Volume&);
            LightFieldReference Union(const std::vector<LightFieldReference>&); //TODO needs merge function
            LightFieldReference Union(const LightFieldReference); //TODO needs merge function
            LightFieldReference Rotate(const angle theta, const angle phi);
            LightFieldReference Partition(const Dimension, const rational);
            LightFieldReference Interpolate(const Dimension, const interpolation::interpolator&);
            LightFieldReference Discretize(const Geometry&);
            LightFieldReference Discretize(const Dimension, const rational);
            LightFieldReference Map(functor&);

        protected:
            explicit Algebra(LightFieldReference &lightField) : this_(lightField) { }

        private:
            const LightFieldReference &this_;
        };
    } // namespace logical
} // namespace lightdb

#endif //LIGHTDB_ALGEBRA_H
