#ifndef LIGHTDB_PYTHON_OPTIONS_H
#define LIGHTDB_PYTHON_OPTIONS_H

#include <any>
#include <unordered_map>
#include <boost/python.hpp>
#include <boost/any.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include "options.h"
#include "Geometry.h"
#include "errors.h"

namespace lightdb::python {
    template<typename TKey=std::string, typename TValue=std::any>
    class PythonOptions {
    public:
        PythonOptions(const boost::python::dict &optDict) {
            boost::python::list keys = boost::python::list(optDict.keys());
            for (int i = 0; i < len(keys); ++i) {
                boost::python::extract<std::string> extractor_keys(keys[i]); 
                std::string key = extractor_keys();
                std::any value = dictToMap(key, optDict);
                _internalMap[key] = value;
            } 
        }

        operator lightdb::options<TKey, TValue>() const { return lightdb::options(_internalMap); }
        
    private:
        std::unordered_map<TKey, TValue> _internalMap;

        static std::any dictToMap(std::string key, boost::python::dict optDict) {
            std::any value = 1;
            if (key == "Volume") {
                boost::python::extract<lightdb::Volume> extractor_values(optDict[key]); 
                value = std::make_any<lightdb::Volume>(extractor_values());
            } else if (key == "Projection") {
                boost::python::extract<lightdb::EquirectangularGeometry> extractor_values(optDict[key]); 
                value = std::make_any<lightdb::GeometryReference>(extractor_values());
            } else if (key == "GOP") {
                boost::python::extract<unsigned int> extractor_values(optDict[key]); 
                value = std::make_any<unsigned int >(extractor_values());
            } else {
                throw InvalidArgumentError("Allowed dictionary keys : Volume, Projection, GOP", key);
            }
            return value;    
        }
    }; 
}

#endif // LIGHTDB_PYTHON_OPTIONS_H