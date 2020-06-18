#include "PythonLightField.h"
#include "PythonGeometry.h"
#include "PythonGreyscale.h"
#include <boost/python.hpp>


namespace lightdb::python {
    BOOST_PYTHON_MODULE (pylightdb) {
        lightdb::optimization::Optimizer::instance<lightdb::optimization::HeuristicOptimizer>(lightdb::execution::LocalEnvironment());

        boost::python::def("Load", +[](const std::string &filepath, boost::python::dict string_options) -> PythonLightField {
            return PythonLightField(lightdb::logical::Load(filepath, PythonOptions{string_options}));
        });
        boost::python::def("Scan", +[](const std::string &name) -> PythonLightField {
            return PythonLightField(lightdb::logical::Scan(name));
        });

        boost::python::class_<lightdb::LightFieldReference>("LightFieldReference", boost::python::no_init);

        boost::python::class_<PythonLightField>("PythonLightField", boost::python::no_init)
                .def("Partition", &PythonLightField::Partition)
                .def("Select", static_cast<PythonLightField(PythonLightField::*)(const lightdb::PhiRange&)>(&PythonLightField::Select))
                .def("Select", static_cast<PythonLightField(PythonLightField::*)(const lightdb::ThetaRange&)>(&PythonLightField::Select))
                .def("Select", static_cast<PythonLightField(PythonLightField::*)(lightdb::SpatiotemporalDimension, const lightdb::SpatiotemporalRange&)>(&PythonLightField::Select))
                .def("Subquery", &PythonLightField::Subquery)
                .def("Union", static_cast<PythonLightField(PythonLightField::*)(PythonLightField&)>(&PythonLightField::Union))
                .def("Union", static_cast<PythonLightField(PythonLightField::*)(boost::python::list&)>(&PythonLightField::Union))
                .def("Discretize", &PythonLightField::Discretize)
                .def("Interpolate", &PythonLightField::Interpolate)
                .def("Map", static_cast<PythonLightField(PythonLightField::*)(PyObject*, const std::filesystem::path&)>(&PythonLightField::Map))
                .def("Map", static_cast<PythonLightField(PythonLightField::*)(const lightdb::functor::unaryfunctor&)>(&PythonLightField::Map))
                .def("Encode", &PythonLightField::Encode)
                .def("Save", &PythonLightField::Save)
                .def("Store", &PythonLightField::Store)
                .def("query", &PythonLightField::query);

        boost::python::class_<lightdb::execution::Coordinator>("Coordinator")
                .def("Execute", static_cast<void(lightdb::execution::Coordinator::*)(const lightdb::LightFieldReference&)>(&lightdb::execution::Coordinator::execute))
                .def("Execute", static_cast<void(lightdb::execution::Coordinator::*)(const lightdb::LightFieldReference&, const lightdb::optimization::Optimizer&)>(&lightdb::execution::Coordinator::execute));

        boost::python::enum_<lightdb::Dimension>("Dimension")
                .value("X", lightdb::Dimension::X)
                .value("Y", lightdb::Dimension::Y)
                .value("Z", lightdb::Dimension::Z)
                .value("Time", lightdb::Dimension::Time)
                .value("Theta", lightdb::Dimension::Theta)
                .value("Phi", lightdb::Dimension::Phi);

        boost::python::enum_<lightdb::SpatiotemporalDimension>("SpatiotemporalDimension")
                .value("X", lightdb::SpatiotemporalDimension::X)
                .value("Y", lightdb::SpatiotemporalDimension::Y)
                .value("Z", lightdb::SpatiotemporalDimension::Z)
                .value("Time", lightdb::SpatiotemporalDimension::Time);

        boost::python::class_<lightdb::Codec>("Codec", boost::python::no_init)
                .def("Hevc", &lightdb::Codec::hevc, boost::python::return_value_policy<boost::python::reference_existing_object>())
                    .staticmethod("Hevc")
                .def("Boxes", &lightdb::Codec::boxes, boost::python::return_value_policy<boost::python::reference_existing_object>())
                    .staticmethod("Boxes");

        boost::python::class_<lightdb::PhiRange>("PhiRange", boost::python::init<double, double>());
        boost::python::class_<lightdb::ThetaRange>("ThetaRange", boost::python::init<double, double>());
        boost::python::class_<lightdb::SpatiotemporalRange>("SpatiotemporalRange", boost::python::init<double, double>());

        boost::python::class_<lightdb::catalog::Catalog>("Catalog", boost::python::init<std::string>());

        boost::python::class_<lightdb::Volume>("Volume", boost::python::init<lightdb::SpatiotemporalRange, lightdb::SpatiotemporalRange, lightdb::SpatiotemporalRange>());
        boost::python::class_<GeometryWrapper, boost::noncopyable>("Geometry", boost::python::no_init);
        boost::python::class_<MeshGeometryWrapper, boost::noncopyable>("MeshGeometry", boost::python::no_init);
        boost::python::class_<lightdb::EquirectangularGeometry, boost::python::bases<lightdb::MeshGeometry>>("EquirectangularGeometry", boost::python::init<std::size_t, std::size_t>());
    
        // Exposing Optimizer is necessary so that HeuristicOptimizer can inherit from it.
        // Don't expose the initializer because Optimizer is an abstract class.
        boost::python::class_<OptimizerWrapper, boost::noncopyable>("Optimizer", boost::python::no_init);
        boost::python::class_<lightdb::optimization::HeuristicOptimizer, boost::python::bases<lightdb::optimization::Optimizer>>("HeuristicOptimizer", boost::python::init<lightdb::execution::Environment>());

        // Exposing Environment is necessary so LocalEnvironment can inherit from it.
        boost::python::class_<lightdb::execution::Environment>("Environment", boost::python::no_init);
        boost::python::class_<lightdb::execution::LocalEnvironment, boost::python::bases<lightdb::execution::Environment>>("LocalEnvironment");

        boost::python::class_<lightdb::interpolation::Linear>("Linear");

        boost::python::class_<lightdb::functor::naryfunctor<1>>("UnaryFunctor", boost::python::no_init);
        boost::python::class_<class lightdb::Greyscale, boost::python::bases<lightdb::functor::unaryfunctor>>("Greyscale");
        boost::python::class_<PythonGreyscale, boost::shared_ptr<PythonGreyscale>, boost::python::bases<lightdb::functor::unaryfunctor>>("PythonGreyscale", boost::python::init<PyObject*>())
                .def(boost::python::init<PyObject*, bool>());
        boost::python::class_<typename lightdb::options<>>("PyOptions", boost::python::no_init);
    };
} // namespace Python
