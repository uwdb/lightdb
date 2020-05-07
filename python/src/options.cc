#include <any>
#include <unordered_map>
#include <boost/python.hpp>
#include <boost/any.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include "options.h"
#include "Geometry.h"



namespace Python {
   // PyOptions
    template<typename TKey=std::string, typename TValue=std::any>
    class PythonOptions {
    public:
        PythonOptions(const boost::python::dict optDict) {
            boost::python::list keys = boost::python::list(optDict.keys());
            for (int i = 0; i < len(keys); ++i) {
                boost::python::extract<std::string> extractor_keys(keys[i]); 
                std::string key = extractor_keys();
                std::any value = 1;
                if (key.compare("Volume") == 0) {
                    boost::python::extract<lightdb::Volume> extractor_values(optDict[key]); 
                    value = std::make_any<lightdb::Volume>(extractor_values());
                } else if (key.compare("Projection") == 0) {
                    boost::python::extract<lightdb::EquirectangularGeometry> extractor_values(optDict[key]); 
                    value = std::make_any<lightdb::GeometryReference>(extractor_values());
                }
                internalMap[key] = value;
            } 
        }

        lightdb::options<> options() {
            return typename lightdb::options<TKey, TValue>::options(internalMap);
        }
        
    private:
        std::unordered_map<TKey, TValue> internalMap;
    }; 
}

