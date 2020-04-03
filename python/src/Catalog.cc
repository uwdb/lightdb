
#include "Algebra.h"
#include "Catalog.h"
#include "Greyscale.h"
#include "HeuristicOptimizer.h"
#include "Visitor.h"
#include "Coordinator.h"
#include "extension.h"
#include "reference.h"
#include "options.h"
#include "LightField.h"
#include "PhysicalOperators.h"
#include "Model.h"
#include "Transaction.h"
#include "Gpac.h"
#include <boost/python.hpp>

namespace Python {

    class PySource {
    public:
        Source(const int index, std::filesystem::path filename, lightdb::Codec codec, const lightdb::Configuration configuration, std::map<std::string, string> options) 
            : _filename(filename),
            _index(index),
            _codec(codec),
            _configuration(configuration),
            _options(options)
            {}  
    private:
        int _index;
        std::filesystem::path _filename;
        lightdb::Codec _codec;
        lightdb::Configuration _configuration;
        std::map<std::string, std::string> _options;
    };


    class PyExternalEntry {
    public:
        ExternalEntry(const std::filesystem::path &filename, std::map<std::string, std::string> options)
            : _filename(filename),
            _options(options)
        {}
    private:
        std::filesystem::path _filename;
        std::map<std::string, std::string> _options;    
    };

    class PyCatalog {
    public:
        Catalog(std::filesystem::path &filename)
            : _filename(filename)
            {}  
    private:
        std::filesystem::path _filename;  

    };

    BOOST_PYTHON_MODULE(pycatalog) {
        boost::python::class_<PySource>("Source", boost::python::no_init);
        boost::python::class_<PyExternalEntry>("ExternalEntry", boost::python::no_init);
        boost::python::class_<PyCatalog>("Catalog", boost::python::no_init);
    };


}


