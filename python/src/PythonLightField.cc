#include "PythonLightField.h"

namespace lightdb::python {
    PythonLightField::PythonLightField(const lightdb::LightFieldReference &lightField)
        : _lightField(lightField)
    { }

    PythonLightField PythonLightField::Partition(lightdb::Dimension dimension, double interval) {
        return PythonLightField(_lightField.Partition(dimension, interval));
    }

    PythonLightField PythonLightField::Select(const lightdb::PhiRange &phiRange) {
        return PythonLightField(_lightField.Select(phiRange));
    }

    PythonLightField PythonLightField::Select(const lightdb::ThetaRange &thetaRange) {
        return PythonLightField(_lightField.Select(thetaRange));
    }

    PythonLightField PythonLightField::Select(lightdb::SpatiotemporalDimension dimension, const lightdb::SpatiotemporalRange &range) {
        return PythonLightField(_lightField.Select(dimension, range));
    }

    PythonLightField PythonLightField::Subquery(PyObject *pyObject) {
        return PythonLightField(_lightField.Subquery([pyObject](auto l) { return boost::python::call<PythonLightField>(pyObject, PythonLightField(l)).query(); }));
    }

    PythonLightField PythonLightField::Union(PythonLightField &lightField) {
        return PythonLightField(_lightField.Union(lightField.query()));
    }

    PythonLightField PythonLightField::Union(boost::python::list &listOfLightFields) {
        std::vector<PythonLightField> pythonLightFields = std::vector<PythonLightField>(
                boost::python::stl_input_iterator<PythonLightField>(listOfLightFields),
                boost::python::stl_input_iterator<PythonLightField>());

        std::vector<lightdb::LightFieldReference> lightFields;
        std::transform(pythonLightFields.begin(), pythonLightFields.end(), std::back_inserter(lightFields),
                [](PythonLightField lightField) -> lightdb::LightFieldReference { return lightField.query(); });

        return PythonLightField(_lightField.Union(lightFields));
    }

    PythonLightField PythonLightField::Discretize(const lightdb::Dimension dimension, double interval) {
        return PythonLightField(_lightField.Discretize(dimension, lightdb::number(interval)));
    }

    PythonLightField PythonLightField::Interpolate(lightdb::Dimension dimension) {
        return PythonLightField(_lightField.Interpolate(dimension, lightdb::interpolation::Linear()));
    }

    PythonLightField PythonLightField::Map(const lightdb::functor::unaryfunctor &functor) {
        return PythonLightField(_lightField.Map(functor));
    }

    PythonLightField PythonLightField::Encode() {
        return PythonLightField(_lightField.Encode());
    }

    PythonLightField PythonLightField::Save(const std::string &fileName) {
        return PythonLightField(_lightField.Save(static_cast<std::filesystem::path>(fileName)));
    }

    PythonLightField PythonLightField::Store(const lightdb::catalog::Catalog &catalog, const std::string &name) {
        return PythonLightField(_lightField.Store(name, catalog));
    }

    lightdb::LightFieldReference PythonLightField::query() {
        return _lightField;
    }

}