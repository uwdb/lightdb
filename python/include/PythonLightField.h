#ifndef LIGHTDB_PYTHON_LIGHT_FIELD_H
#define LIGHTDB_PYTHON_LIGHT_FIELD_H

#include "Algebra.h"
#include "Catalog.h"
#include "Greyscale.h"
#include "Visitor.h"
#include "Coordinator.h"
#include "LightField.h"
#include "extension.h"
#include "reference.h"
#include "PythonOptimizer.h"
#include "PythonOptions.h"

namespace lightdb::python {

    class PythonLightField {
    public:
        explicit PythonLightField(const lightdb::LightFieldReference &lightField);
        PythonLightField Partition(lightdb::Dimension dimension, double interval);
        PythonLightField Select(const lightdb::PhiRange &phiRange);
        PythonLightField Select(const lightdb::ThetaRange &thetaRange);
        PythonLightField Select(lightdb::SpatiotemporalDimension dimension, const lightdb::SpatiotemporalRange &range);
        PythonLightField Subquery(PyObject *PyObject);
        PythonLightField Union(PythonLightField &lightField);
        PythonLightField Union(boost::python::list &listOfLightFields);
        PythonLightField Discretize(lightdb::Dimension dimension, double interval);
        PythonLightField Interpolate(lightdb::Dimension dimension);
        PythonLightField Map(const lightdb::functor::unaryfunctor &functor);
        PythonLightField Encode();
        PythonLightField Save(const std::string &filename);
        PythonLightField Store(const lightdb::catalog::Catalog &catalog, const std::string &name);
        lightdb::LightFieldReference query();

    private:
        lightdb::LightFieldReference _lightField;       
    };
}

#endif //LIGHTDB_PYTHON_LIGHT_FIELD_H