#include <boost/python.hpp>
#include "PythonLightField.h"
#include "number.h"


namespace lightdb::python {

void (lightdb::execution::Coordinator::*ExecuteLightField)(const lightdb::LightFieldReference&) = &lightdb::execution::Coordinator::execute;
void (lightdb::execution::Coordinator::*ExecuteLightFieldAndOptimizer)(const lightdb::LightFieldReference&, const lightdb::optimization::Optimizer&) = &lightdb::execution::Coordinator::execute;


static void SetUpLocalEnvironment() {
    lightdb::optimization::Optimizer::instance<lightdb::optimization::HeuristicOptimizer>(lightdb::execution::LocalEnvironment());
}

static PythonLightField Load(const std::string &filepath, boost::python::dict string_options) {
PythonOptions opt(string_options);
return PythonLightField(lightdb::logical::Load(filepath, opt.options()));
}

static PythonLightField Scan(const std::string &name) {
    return PythonLightField(lightdb::logical::Scan(name));
}


// Create wrappers that implements pure virtual function to make boost happy.

class OptimizerWrapper : public lightdb::optimization::Optimizer, public boost::python::wrapper<lightdb::optimization::Optimizer> {
protected:
    const std::vector<std::shared_ptr<lightdb::optimization::OptimizerRule>> rules() const {
        return this->get_override("rules")();
    }
};

class GeometryWrapper : public lightdb::Geometry, public boost::python::wrapper<lightdb::Geometry>  {
    public:
        bool is_monotonic() const {
            return this->get_override("is_monotonic")();
        }
        bool defined_at(const lightdb::Point6D &point) const {
            return this->get_override("defined_at")();
        }
};

typedef lightdb::number angle;
class MeshGeometryWrapper : public lightdb::MeshGeometry, public boost::python::wrapper<lightdb::MeshGeometry> {
    public:
        double u(angle theth, angle phi) const {
            return this->get_override("u")();
        }
        double v(angle theth, angle phi) const {
            return this->get_override("v")();
        }
};


//#pragma GCC visibility push(default)
BOOST_PYTHON_MODULE (pylightdb) {
    boost::python::def("Load", Load);
    boost::python::def("Scan", Scan);
    boost::python::def("SetUpLocalEnvironment", SetUpLocalEnvironment);

    boost::python::class_<lightdb::LightFieldReference>("LightFieldReference", boost::python::no_init);

    boost::python::class_<PythonLightField>("PythonLightField", boost::python::no_init)
            .def("Partition", &PythonLightField::Partition)
            .def("Select", &PythonLightField::SelectPhi)
            .def("Select", &PythonLightField::SelectTheta)
            .def("Select", &PythonLightField::SelectSpatiotemporal)
            .def("Subquery", &PythonLightField::Subquery)
            .def("Union", &PythonLightField::UnionOne)
            .def("Union", &PythonLightField::UnionMany)
            .def("Discretize", &PythonLightField::Discretize)
            .def("Interpolate", &PythonLightField::Interpolate)
            .def("Map", &PythonLightField::PythonMap)
            .def("Map", &PythonLightField::FunctorMap)
            .def("Encode", &PythonLightField::Encode)
            .def("Save", &PythonLightField::Save)
            .def("Store", &PythonLightField::Store)
            .def("query", &PythonLightField::query);

    boost::python::class_<lightdb::execution::Coordinator>("Coordinator")
            .def("Execute", ExecuteLightField)
            .def("Execute", ExecuteLightFieldAndOptimizer);

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

    boost::python::class_<typename lightdb::options<>>("PyOptions", boost::python::no_init);        
        
};
//BH #pragma GCC visibility pop
} // namespace Python
