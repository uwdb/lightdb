#include "Algebra.h"
#include "Catalog.h"
#include "Greyscale.h"
#include "HeuristicOptimizer.h"
#include "Visitor.h"
#include "Coordinator.h"
#include "extension.h"
#include "reference.h"
#include "options.h"
#include <boost/python.hpp>
#include <boost/any.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include "Dict2Map.hh"


namespace Python {

void (lightdb::execution::Coordinator::*ExecuteLightField)(const lightdb::LightFieldReference&) = &lightdb::execution::Coordinator::execute;
void (lightdb::execution::Coordinator::*ExecuteLightFieldAndOptimizer)(const lightdb::LightFieldReference&, const lightdb::optimization::Optimizer&) = &lightdb::execution::Coordinator::execute;


static void SetUpLocalEnvironment() {
    lightdb::optimization::Optimizer::instance<lightdb::optimization::HeuristicOptimizer>(lightdb::execution::LocalEnvironment());
}

class PythonLightField {
public:
    PythonLightField(const lightdb::LightFieldReference &lightField)
        : _lightField(lightField)
    {}

    PythonLightField Partition(lightdb::Dimension dimension, const double interval) {
        return PythonLightField(_lightField.Partition(dimension, interval));
    }

    PythonLightField Select(const lightdb::PhiRange &phiRange) {
        return PythonLightField(_lightField.Select(phiRange));
    }

    PythonLightField Select(const lightdb::ThetaRange &thetaRange) {
        return PythonLightField(_lightField.Select(thetaRange));
    }

    PythonLightField Select(lightdb::SpatiotemporalDimension dimension, const lightdb::SpatiotemporalRange &range) {
        return PythonLightField(_lightField.Select(dimension, range));
    }

    PythonLightField Subquery(PyObject *pyObject) {
        return PythonLightField(_lightField.Subquery([pyObject](auto l) { return boost::python::call<PythonLightField>(pyObject, PythonLightField(l)).query(); }));
    }

    PythonLightField Union(PythonLightField &lightField) {
        return PythonLightField(_lightField.Union(lightField.query()));
    }

    PythonLightField Union(boost::python::list &listOfLightFields) {
        std::vector<PythonLightField> pythonLightFields = std::vector<PythonLightField>(
                boost::python::stl_input_iterator<PythonLightField>(listOfLightFields),
                boost::python::stl_input_iterator<PythonLightField>());

        std::vector<lightdb::LightFieldReference> lightFields;
        std::transform(pythonLightFields.begin(), pythonLightFields.end(), std::back_inserter(lightFields),
                [](PythonLightField lightField) -> lightdb::LightFieldReference { return lightField.query(); });

        return PythonLightField(_lightField.Union(lightFields));
    }

    PythonLightField Discretize(lightdb::Dimension dimension, double interval) {
        return PythonLightField(_lightField.Discretize(dimension, lightdb::number(interval)));
    }

    PythonLightField Interpolate(lightdb::Dimension dimension) {
        return PythonLightField(_lightField.Interpolate(dimension, lightdb::interpolation::Linear()));
    }

    PythonLightField Map(PyObject *udf, std::string path) {
        auto yolo = lightdb::extensibility::Load("yolo", path);
        return PythonLightField(_lightField.Map(yolo));
    }

    PythonLightField Map(lightdb::functor::unaryfunctor functor) {
        return PythonLightField(_lightField.Map(functor));
    }

    PythonLightField Encode() {
        return PythonLightField(_lightField.Encode());
    }

    PythonLightField Save(const std::string &fileName) {
        return PythonLightField(_lightField.Save(static_cast<std::filesystem::path>(fileName)));
    }

    PythonLightField Store(const lightdb::catalog::Catalog &catalog, const std::string &name) {
        return PythonLightField(_lightField.Store(name, catalog));
    }

    lightdb::LightFieldReference query() {
        return _lightField;
    }

private:
    lightdb::LightFieldReference _lightField;
};

PythonLightField (PythonLightField::*SelectPhi)(const lightdb::PhiRange&) = &PythonLightField::Select;
PythonLightField (PythonLightField::*SelectTheta)(const lightdb::ThetaRange&) = &PythonLightField::Select;
PythonLightField (PythonLightField::*SelectSpatiotemporal)(lightdb::SpatiotemporalDimension, const lightdb::SpatiotemporalRange&) = &PythonLightField::Select;
PythonLightField (PythonLightField::*UnionOne)(PythonLightField&) = &PythonLightField::Union;
PythonLightField (PythonLightField::*UnionMany)(boost::python::list&) = &PythonLightField::Union;
PythonLightField (PythonLightField::*PythonMap)(PyObject*, std::string) = &PythonLightField::Map;
PythonLightField (PythonLightField::*FunctorMap)(lightdb::functor::unaryfunctor) = &PythonLightField::Map;

static PythonLightField Load(const std::string &filepath, boost::python::dict mydict) {
    lightdb::GeometryReference geometry = lightdb::GeometryReference::make<lightdb::EquirectangularGeometry>(lightdb::EquirectangularGeometry::Samples());
    std::map<std::string, std::string> myMap;
    boost::python::list keys = boost::python::list(mydict.keys());
    for (int i = 0; i < len(keys); ++i) {
        boost::python::extract<std::string> extractor_keys(keys[i]);
        if (!extractor_keys.check()) {  
                std::cout<<"Key invalid, map might be incomplete"<<std::endl;  
                continue;                 
        }  
        std::string key = extractor_keys();
        boost::python::extract<std::string> extractor_values(mydict[key]);
        if (!extractor_values.check()) {  
           std::cout<<"Value invalid, map might be incomplete"<<std::endl;  
                continue;                 
        }  
        std::string value = extractor_values();
        myMap[key] = value;
    }
    // std::cout<<myMap<<std::endl; 
    return PythonLightField(lightdb::logical::Load(filepath, lightdb::Volume::angular(), geometry));
}

static PythonLightField Scan(const std::string &name) {
    return PythonLightField(lightdb::logical::Scan(name));
}

// Create wrapper that implements pure virtual function to make boost happy.
class OptimizerWrapper : public lightdb::optimization::Optimizer, public boost::python::wrapper<lightdb::optimization::Optimizer> {
protected:
    const std::vector<std::shared_ptr<lightdb::optimization::OptimizerRule>> rules() const {
        return this->get_override("rules")();
    }
};

// typedef std::map<std::string, boost::any> mymap_t;
// class OptionsDict {
// public:
//     OptionsDict(cont mymap_t& m) {}
//     std::map getMap() {
//         return m;
//     }
// };

BOOST_PYTHON_MODULE (pylightdb) {
    boost::python::def("Load", Load);
    boost::python::def("Scan", Scan);
    boost::python::def("SetUpLocalEnvironment", SetUpLocalEnvironment);

    boost::python::class_<lightdb::LightFieldReference>("LightFieldReference", boost::python::no_init);

    boost::python::class_<PythonLightField>("PythonLightField", boost::python::no_init)
            .def("Partition", &PythonLightField::Partition)
            .def("Select", SelectPhi)
            .def("Select", SelectTheta)
            .def("Select", SelectSpatiotemporal)
            .def("Subquery", &PythonLightField::Subquery)
            .def("Union", UnionOne)
            .def("Union", UnionMany)
            .def("Discretize", &PythonLightField::Discretize)
            .def("Interpolate", &PythonLightField::Interpolate)
            .def("Map", PythonMap)
            .def("Map", FunctorMap)
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

    // volume  
    boost::python::class_<lightdb::Volume>("Volume", boost::python::init<lightdb::SpatiotemporalRange, lightdb::SpatiotemporalRange, lightdb::SpatiotemporalRange>());
    boost::python::class_<lightdb::Point3D>("Point3D", boost::python::init<double, double, double>());
    // boost::python::class_<lightdb::Point2D>("Point2D", boost::python::init<double, double>());
    // boost::python::class_<lightdb::options>("options", boost::python::init<std::map<std::string, std::string>>());
    // boost::python::class_<lightdb::GOP>("GOP", boost::python::init<int>());
    // GOPszie is unsigned int, 0 is invalid

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
    
};
} // namespace Python
