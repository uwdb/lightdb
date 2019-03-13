#ifndef LIGHTDB_STOREOPERATORS_H
#define LIGHTDB_STOREOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {

class Store: public PhysicalLightField, public EncodedVideoInterface {
public:
    explicit Store(const LightFieldReference &logical,
                   PhysicalLightFieldReference &parent)
            : Store(logical,
                    logical->downcast<logical::StoredLightField>(),
                    parent)
    { }

    const Codec &codec() const override { return store_.codec(); }
    //const Configuration &configuration() override { return configuration_.value(); }

private:
    explicit Store(const LightFieldReference &logical,
                   const logical::StoredLightField &store,
                   PhysicalLightFieldReference &parent)
            : PhysicalLightField(logical, {parent}, DeviceType::CPU, runtime::make<Runtime>(*this, store)),
              store_(store)
              //parent_(parent)
              /*configuration_([this]() {
                  if(parent_.is<EncodedVideoInterface>())
                      return parent_.downcast<EncodedVideoInterface>().configuration();
                  else {
                      LOG(ERROR) << "Using stub configuration because CPUOperators don't yet carry a configuration instance.  This is horrible."; //TODO
                      return Configuration{1, 1, 0, 0, 0, {1, 1}, {}};
                  }
              })*/ {
              //output_([this]() { return catalog::Catalog::instance().create(store_.name(), store_.codec(), configuration_); }) {
              //output_([this]() { return catalog::Catalog::instance().create(store_.name(), parent_.downcast<EncodedVideoInterface>().codec(), configuration_); }) {
        LOG(INFO) << "Storing to ambient catalog";
        CHECK_EQ(parents().size(), 1);
        //CHECK(parent_.is<EncodedVideoInterface>()) << "Store only supports internally persisting encoded video.";
    }

    class Runtime: public runtime::UnaryRuntime<Store, CPUEncodedFrameData> {
    public:
        explicit Runtime(Store &physical, const logical::StoredLightField &logical)
            : runtime::UnaryRuntime<Store, CPUEncodedFrameData>(physical),
              output_(catalog::Catalog::instance().create(
                      logical.name(),
                      logical.codec(),
                      tempConfiguration()))
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(!all_parent_eos()) {
                //auto data = iterators()[0]++;
                //auto &encoded = data.downcast<physical::CPUEncodedFrameData>();
                auto data = iterator()++;
                std::copy(data.value().begin(),data.value().end(), std::ostreambuf_iterator<char>(output_.stream()));
                return data;
            } else
                return std::nullopt;
        }

    private:
        Configuration tempConfiguration() {
            auto &it = iterators().front();
            auto current = *it;

            return current.downcast<FrameData>().configuration();
            /*
            if(physical().parents().front().is<EncodedVideoInterface>())
                return physical().parents().front().downcast<EncodedVideoInterface>().configuration();
            else {
                LOG(ERROR) << "Using stub configuration because CPUOperators don't yet carry a configuration instance.  This is horrible."; //TODO
                return Configuration{1, 1, 0, 0, 0, {1, 1}, {}};
            }*/
        }

        catalog::OutputStream output_;
    };

    const logical::StoredLightField &store_;
    //PhysicalLightFieldReference parent_;
    //lazy<Configuration> configuration_;
};

} // namespace lightdb::physical

#endif //LIGHTDB_STOREOPERATORS_H
