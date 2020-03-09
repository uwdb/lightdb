#include "boost/python.hpp"
namespace bpy = boost::python;

/// This template encapsulates the conversion machinery.
template<typename key_t, typename val_t>
struct Dict2Map {

    /// The type of the map we convert the Python dict into
    typedef std::map<key_t, val_t> map_t;

    /// constructor
    /// registers the converter with the Boost.Python runtime
    Dict2Map() {
        bpy::converter::registry::push_back(
            &convertible,
            &construct,
            bpy::type_id<map_t>()
#ifdef BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
            , &bpy::converter::wrap_pytype<&PyDict_Type>::get_pytype
#endif
        );
    }

    /// Check if conversion is possible
    static void* convertible(PyObject* objptr) {
        return PyDict_Check(objptr)? objptr: nullptr;
    }

    /// Perform the conversion
    static void construct(
        PyObject* objptr,
        bpy::converter::rvalue_from_python_stage1_data* data
    ) {
        // convert the PyObject pointed to by `objptr` to a bpy::dict
        bpy::handle<> objhandle{ bpy::borrowed(objptr) };   // "smart ptr"
        bpy::dict d{ objhandle };

        // get a pointer to memory into which we construct the map
        // this is provided by the Python runtime
        void* storage = 
            reinterpret_cast<
                bpy::converter::rvalue_from_python_storage<map_t>*
            >(data)->storage.bytes;

        // placement-new allocate the result
        new(storage) map_t{};

        // iterate over the dictionary `d`, fill up the map `m`
        map_t& m{ *(static_cast<map_t *>(storage)) };
        bpy::list keys{ d.keys() };
        int keycount{ static_cast<int>(bpy::len(keys)) };
        for (int i = 0; i < keycount; ++i) {
            // get the key
            bpy::object keyobj{ keys[i] };
            bpy::extract<key_t> keyproxy{ keyobj };
            if (! keyproxy.check()) {
                PyErr_SetString(PyExc_KeyError, "Bad key type");
                bpy::throw_error_already_set();
            }
            key_t key = keyproxy();

            // get the corresponding value
            bpy::object valobj{ d[keyobj] };
            bpy::extract<val_t> valproxy{ valobj };
            if (! valproxy.check()) {
                PyErr_SetString(PyExc_ValueError, "Bad value type");
                bpy::throw_error_already_set();
            }
            val_t val = valproxy();
            m[key] = val;
        }

        // remember the location for later
        data->convertible = storage;
    }
};