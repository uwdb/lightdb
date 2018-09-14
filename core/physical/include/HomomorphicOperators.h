#ifndef LIGHTDB_HOMOMORPHICOPERATORS_H
#define LIGHTDB_HOMOMORPHICOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class HomomorphicUniformAngularUnion: public PhysicalLightField, public EncodedVideoInterface {
public:
    explicit HomomorphicUniformAngularUnion(const LightFieldReference &logical,
                                            std::vector<PhysicalLightFieldReference> &parents,
                                            const unsigned int rows, const unsigned int columns)
            : PhysicalLightField(logical, parents, DeviceType::CPU),
              rows_(rows), columns_(columns),
              configuration_(get_configuration(parents, rows_, columns_))
    {
        LOG(WARNING) << "Ignored myriad preconditions on homomorphic angular union";
    }

    std::ofstream tmpfile[64];


    std::optional<physical::MaterializedLightFieldReference> read() override {
        if(!any_parent_eos()) {
            std::optional<physical::MaterializedLightFieldReference> data;
            static auto tmpindex = 0u;

            if(tmpindex == 0u) {
                for(auto i = 0u; i < iterators().size(); i++)
                    tmpfile[i] = std::ofstream(std::string("homoout") + std::to_string(i) + ".hevc");
            }

            for(auto &it: iterators()) {
                data = it++;
                tmpindex++;

                if(data.has_value()) {
                    auto &encoded = data.value().downcast<CPUEncodedFrameData>();
                    printf("hout %lu\n", encoded.value().size());
                    std::copy(encoded.value().begin(),encoded.value().end(),std::ostreambuf_iterator<char>(tmpfile[tmpindex % iterators().size()]));
                }
            }
            return data;
        }
        return {};
    }

    const Codec &codec() const override { return Codec::hevc(); }
    const Configuration &configuration() const override { return configuration_; }

private:
    const unsigned int rows_, columns_;
    const Configuration configuration_;

    static const Configuration get_configuration(
            std::vector<PhysicalLightFieldReference>& sources,
            const unsigned int rows, const unsigned int columns) {
        auto &configuration = sources[0].downcast<GPUOperator>().configuration();

        //TODO assert sources[1...n] preconditions (e.g., have same width, height, framerate)
        LOG(WARNING) << "Did not assert preconditions on homomorphic angular union sources";

        return Configuration{configuration.width * columns,
                             configuration.height * rows,
                             configuration.max_width * columns,
                             configuration.max_height * rows,
                             configuration.bitrate,
                             configuration.framerate};
    }
};

} // namespace lightdb::physical

#endif //LIGHTDB_HOMOMORPHICOPERATORS_H
