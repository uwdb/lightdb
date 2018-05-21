#ifndef LIGHTDB_HOMOMORPHICOPERATORS_H
#define LIGHTDB_HOMOMORPHICOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class HomomorphicUniformTile: public PhysicalLightField, public EncodedVideoInterface {
public:
    explicit HomomorphicUniformTile(const LightFieldReference &logical,
                                    const std::vector<PhysicalLightFieldReference> &parents,
                                    const unsigned int rows, const unsigned int columns)
            : PhysicalLightField(logical, parents, DeviceType::CPU),
              rows_(rows), columns_(columns),
              sources_(functional::transform<std::reference_wrapper<const EncodedVideoInterface>>(
                      parents.begin(), parents.end(), [](const PhysicalLightFieldReference &parent) {
                          return std::ref(parent.downcast<EncodedVideoInterface>()); })),
              configuration_{get_configuration(sources_, rows_, columns_)}
    {
        //TODO assert preconditions
        throw NotImplementedError();
    }

    std::optional<physical::DataReference> read() override {
        //auto chunks = functional::transform(sources_.begin(), sources_.end(), [](auto &source) {
        //                                        return source.read(); });
        throw NotImplementedError();
    }

    const Codec &codec() const override { return Codec::hevc(); }
    const Configuration &configuration() const override { return configuration_; }

private:
    const unsigned int rows_, columns_;
    const std::vector<std::reference_wrapper<const EncodedVideoInterface>> sources_;
    const Configuration configuration_;

    static const Configuration get_configuration(
            const std::vector<std::reference_wrapper<const EncodedVideoInterface>>& sources,
            const unsigned int rows, const unsigned int columns) {
        auto &first_configuration = sources[0].get().configuration();

        //TODO assert sources[1...n] preconditions (e.g., have same width, height, framerate)

        return Configuration{first_configuration.width * columns,
                             first_configuration.height * rows,
                             first_configuration.max_width * columns,
                             first_configuration.max_height * rows,
                             0,
                             first_configuration.framerate};
    }
};

} // namespace lightdb::physical

#endif //LIGHTDB_HOMOMORPHICOPERATORS_H
