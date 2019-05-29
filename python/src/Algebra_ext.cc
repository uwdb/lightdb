#include "Algebra.h"
//
#include "Catalog.h"
#include "HeuristicOptimizer.h"
#include "Visitor.h"
#include "reference.h"
#include <boost/python.hpp>
#include <cassert>

// This is following a different code path because it's an external light field. Does this matter?
// Seems to matter. Test case in SelectionTests fails when run with Load() instead of Scan().
// Need to implement something for catalogs.

namespace Python {

//lightdb::LightFieldReference (*Scan1)(const std::string&) = &lightdb::logical::Scan;
//lightdb::LightFieldReference (*Scan2)(const lightdb::catalog::Catalog&, const std::string&) = &lightdb::logical::Scan;

class PyAlgebra {
public:
    PyAlgebra(lightdb::LightFieldReference &lightField)
            : lightField_(lightField)
    {
        lightdb::optimization::Optimizer::instance<lightdb::optimization::HeuristicOptimizer>(lightdb::execution::LocalEnvironment());
    }

    PyAlgebra Save(std::string pathAsString) {
        lightdb::LightFieldReference savedLightField = lightField_.Save(pathAsString);
        return PyAlgebra(savedLightField);
    }

    PyAlgebra Encode() {
        lightdb::LightFieldReference encodedLightField = lightField_.Encode();
        return PyAlgebra(encodedLightField);
    }

    PyAlgebra Select(const lightdb::PhiRange &phiRange) {
        lightdb::LightFieldReference selectedLightField = lightField_.Select(phiRange);
        return PyAlgebra(selectedLightField);
    }

    PyAlgebra SelectPhi(int numerator, int denominator) {
        lightdb::rational_times_real end = lightdb::rational_times_real({numerator, denominator}, lightdb::PI);
        lightdb::LightFieldReference selectedLightField = lightField_.Select(lightdb::PhiRange(0, end));
        return PyAlgebra(selectedLightField);
    }

    void Execute() {
        lightdb::optimization::Optimizer::instance<lightdb::optimization::HeuristicOptimizer>(lightdb::execution::LocalEnvironment());
        lightdb::execution::Coordinator().execute(lightField_);
    }

private:
    lightdb::LightFieldReference lightField_;
};

static PyAlgebra Load(std::string pathAsString) {
    lightdb::LightFieldReference loadedLightField = lightdb::logical::Load(pathAsString);
    return PyAlgebra(loadedLightField);
}

static PyAlgebra Scan(std::string resourceName) {
    lightdb::catalog::Catalog catalog = lightdb::catalog::Catalog("/home/maureen/lightdb/test/resources/");
    lightdb::catalog::Catalog::instance(catalog);
    lightdb::LightFieldReference scannedLightField = lightdb::logical::Scan(resourceName);
    return PyAlgebra(scannedLightField);
}

BOOST_PYTHON_MODULE (algebra_ext) {
    boost::python::def("Load", Load);
    boost::python::def("Scan", Scan);

    boost::python::class_<PyAlgebra>("LightField", boost::python::no_init)
            .def("Save", &PyAlgebra::Save)
            .def("Encode", &PyAlgebra::Encode)
            .def("Execute", &PyAlgebra::Execute)
            .def("Select", &PyAlgebra::Select)
            .def("SelectPhi", &PyAlgebra::SelectPhi);

    boost::python::class_<lightdb::number>("number", boost::python::init<int>())
            .def(boost::python::init<long>())
            .def(boost::python::init<long long>())
            .def(boost::python::init<unsigned int>())
            .def(boost::python::init<double>())
            .def(boost::python::self_ns::str(boost::python::self_ns::self));

    boost::python::class_<lightdb::PhiRange>("PhiRange", boost::python::init<lightdb::number, lightdb::number>())
            .def("start", &lightdb::PhiRange::start)
            .def("end", &lightdb::PhiRange::end)
            .def("contains", &lightdb::PhiRange::contains);
}
} // namespace Python
