#include "Algebra.h"
//
#include "Catalog.h"
#include "Visitor.h"
#include <boost/python.hpp>

lightdb::LightFieldReference (*Scan1)(const std::string&) = &lightdb::logical::Scan;
lightdb::LightFieldReference (*Scan2)(const lightdb::catalog::Catalog&, const std::string&) = &lightdb::logical::Scan;

BOOST_PYTHON_MODULE(algebra_ext) {
    boost::python::def("Scan", Scan1);
    boost::python::def("Scan", Scan2);
}
