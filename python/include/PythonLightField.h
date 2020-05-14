#ifndef LIGHTDB_PYTHON_LIGHT_FIELD_H
#define LIGHTDB_PYTHON_LIGHT_FIELD_H
#include "pyoptions.h"
#include "Algebra.h"
#include "Catalog.h"
#include "Greyscale.h"
#include "HeuristicOptimizer.h"
#include "Visitor.h"
#include "Coordinator.h"
#include "extension.h"
#include "reference.h"
#include "LightField.h"

namespace LightDB::Python {

    class PythonLightField {
    public:
        PythonLightField(const lightdb::LightFieldReference &lightField);
        PythonLightField Partition(lightdb::Dimension dimension, const double interval);
        PythonLightField SelectPhi(const lightdb::PhiRange &phiRange);
        PythonLightField SelectTheta(const lightdb::ThetaRange &thetaRange);
        PythonLightField SelectSpatiotemporal(lightdb::SpatiotemporalDimension dimension, const lightdb::SpatiotemporalRange &range);
        PythonLightField Subquery(PyObject *PyObject);
        PythonLightField UnionOne(PythonLightField &lightField);
        PythonLightField UnionMany(boost::python::list &listOfLightFields);
        PythonLightField Discretize(const lightdb::Dimension dimension, double interval);
        PythonLightField Interpolate(lightdb::Dimension dimension);
        PythonLightField PythonMap(PyObject *udf, std::filesystem::path path);
        PythonLightField FunctorMap(lightdb::functor::unaryfunctor functor);
        PythonLightField Encode();
        PythonLightField Save(const std::string &filename);
        PythonLightField Store(const lightdb::catalog::Catalog &catalog, const std::string &name);
        lightdb::LightFieldReference query();
    private:
        lightdb::LightFieldReference _lightField;       
    };
}

#endif //LIGHTDB_PYTHON_LIGHT_FIELD_H