#ifndef LIGHTDB_SERIALIZATION_H
#define LIGHTDB_SERIALIZATION_H

#include "Metadata.pb.h"
#include "Geometry.h"
#include "errors.h"

namespace lightdb::serialization {
    GeometryReference as_geometry(const serialization::Metadata::Entry &entry) {
        switch(entry.projection_case()) {
            case serialization::Metadata::Entry::ProjectionCase::kEquirectangular: {
                auto samples = entry.equirectangular().samples();
                return GeometryReference::make<EquirectangularGeometry>(EquirectangularGeometry::Samples{
                        samples.x(), samples.y()});
            }

            case serialization::Metadata::Entry::ProjectionCase::PROJECTION_NOT_SET:
                throw SerializationError("Serialized projection type missing");

            default:
                throw SerializationError("Invalid serialized projection type");
        }
    }

    Volume as_volume(const serialization::Metadata::Volume &volume) {
        return Volume{{volume.x1(), volume.x2()},
                      {volume.y1(), volume.y2()},
                      {volume.z1(), volume.z2()},
                      {volume.t1(), volume.t2()},
                      {volume.theta1(), volume.theta2()},
                      {volume.phi1(), volume.phi2()} };
    }

    CompositeVolume as_composite_volume(const serialization::Metadata::Entry &entry) {
        std::vector<Volume> volumes;
        std::transform(entry.partitions().begin(), entry.partitions().end(),
                       std::back_inserter(volumes),
                       [](auto &partition) { return as_volume(partition); });
        return CompositeVolume{volumes};
    }

    serialization::Metadata::Volume as_partition(const Volume &volume) {
        serialization::Metadata::Volume serialized_volume;

        serialized_volume.set_x1(static_cast<double>(volume.x().start()));
        serialized_volume.set_x2(static_cast<double>(volume.x().end()));
        serialized_volume.set_y1(static_cast<double>(volume.y().start()));
        serialized_volume.set_y2(static_cast<double>(volume.y().end()));
        serialized_volume.set_z1(static_cast<double>(volume.z().start()));
        serialized_volume.set_z2(static_cast<double>(volume.z().end()));
        serialized_volume.set_t1(static_cast<double>(volume.t().start()));
        serialized_volume.set_t2(static_cast<double>(volume.t().end()));
        serialized_volume.set_theta1(static_cast<double>(volume.theta().start()));
        serialized_volume.set_theta2(static_cast<double>(volume.theta().end()));
        serialized_volume.set_phi1(static_cast<double>(volume.phi().start()));
        serialized_volume.set_phi2(static_cast<double>(volume.phi().end()));

        return serialized_volume;
    }

    void set_geometry(serialization::Metadata::Entry &entry, const transactions::OutputStream &stream) {
        if(stream.geometry().is<EquirectangularGeometry>()) {
            auto geometry = stream.geometry().downcast<EquirectangularGeometry>();
            auto message = entry.mutable_equirectangular();
            message->mutable_samples()->set_x(geometry.samples().theta);
            message->mutable_samples()->set_y(geometry.samples().phi);
        } else
            throw SerializationError("Serialization of geometry not supported");
    }

    serialization::Metadata::Entry as_entry(const transactions::OutputStream &stream) {
        serialization::Metadata::Entry entry;

        entry.set_type(stream.volume().bounding().is_point()
            ? serialization::Metadata::POINT
            : serialization::Metadata::PLANE);

        for(const auto &volume: stream.volume().components()) {
            auto partition = entry.add_partitions();
            partition->CopyFrom(as_partition(volume));
        }

        set_geometry(entry, stream);

        return entry;
    }
} // namespace lightdb::serialization

#endif //LIGHTDB_SERIALIZATION_H
