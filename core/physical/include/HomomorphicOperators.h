#ifndef LIGHTDB_HOMOMORPHICOPERATORS_H
#define LIGHTDB_HOMOMORPHICOPERATORS_H

#include "PhysicalOperators.h"
#include "Stitcher.h"

namespace lightdb::physical {

class HomomorphicUniformAngularUnion: public PhysicalLightField /*, public EncodedVideoInterface*/ {
public:
    explicit HomomorphicUniformAngularUnion(const LightFieldReference &logical,
                                            std::vector<PhysicalLightFieldReference> &parents,
                                            const unsigned int rows, const unsigned int columns)
            : PhysicalLightField(logical, parents, DeviceType::CPU, runtime::make<Runtime>(*this)),
              rows_(rows), columns_(columns) {
        LOG(WARNING) << "Ignored myriad preconditions on homomorphic angular union";
    }

    unsigned int rows() const { return rows_; }
    unsigned int columns() const { return columns_;}

private:
    class Runtime: public runtime::Runtime<HomomorphicUniformAngularUnion> {
    public:
        explicit Runtime(HomomorphicUniformAngularUnion &physical)
            : runtime::Runtime<HomomorphicUniformAngularUnion>(physical),
              materializedData_(physical.rows() * physical.columns()),
              configuration_(create_configuration())
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(!any_parent_eos()) {

                // Materialize everything :(
                for (auto index = 0u; index < iterators().size(); index++)
                    if (iterators()[index] != iterators()[index].eos()) {
                        auto next = (iterators()[index]++);
                        const auto &data = next.downcast<CPUEncodedFrameData>().value();
                        materializedData_[index].insert(std::end(materializedData_[index]), data.begin(), data.end());
                    }

                return CPUEncodedFrameData(Codec::hevc(), configuration_, bytestring{});
            } else if(!materializedData_.empty()) {
                lightdb::hevc::Context context({physical().rows(), physical().columns()},
                                               {configuration_.height / physical().rows(),
                                                configuration_.width / physical().columns()});
                lightdb::hevc::Stitcher stitcher(context, materializedData_);
                materializedData_.clear();

                return CPUEncodedFrameData(Codec::hevc(), configuration_, stitcher.GetStitchedSegments());
            } else
                return {};
        }

    private:
        const Configuration create_configuration() {
            auto configuration = (*iterators().front()).downcast<FrameData>().configuration();

            //TODO assert sources[1...n] preconditions (e.g., have same width, height, framerate)
            LOG(WARNING) << "Did not assert preconditions on homomorphic angular union sources";

            return Configuration{configuration.width * physical().columns(),
                                 configuration.height * physical().rows(),
                                 configuration.max_width * physical().columns(),
                                 configuration.max_height * physical().rows(),
                                 configuration.bitrate,
                                 configuration.framerate, {}};
        }

        std::vector<bytestring> materializedData_;
        const Configuration configuration_;
    };

    const unsigned int rows_, columns_;
};

} // namespace lightdb::physical

#endif //LIGHTDB_HOMOMORPHICOPERATORS_H
