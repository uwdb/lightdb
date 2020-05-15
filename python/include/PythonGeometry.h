#ifndef LIGHTDB_PYTON_GEOMETRY_H
#define LIGHTDB_PYTON_GEOMETRY_H

#include <boost/python.hpp>
#include "Geometry.h"

namespace lightdb::python {
    class GeometryWrapper : public lightdb::Geometry, public boost::python::wrapper<lightdb::Geometry>  {
    public:
        bool is_monotonic() const {
            return this->get_override("is_monotonic")();
        }
        bool defined_at(const lightdb::Point6D &point) const {
            return this->get_override("defined_at")();
        }
    };

    using angle = lightdb::number;
    class MeshGeometryWrapper : public lightdb::MeshGeometry, public boost::python::wrapper<lightdb::MeshGeometry> {
        public:
            double u(angle theta, angle phi) const {
                return this->get_override("u")();
            }
            double v(angle theta, angle phi) const {
                return this->get_override("v")();
            }
    };
}


#endif //LIGHTDB_PYTON_GEOMETRY_H