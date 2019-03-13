#ifndef LIGHTDB_PHYSICALOPERATORS_H
#define LIGHTDB_PHYSICALOPERATORS_H

#include "LightField.h"
#include "Encoding.h"
#include "Environment.h"
#include "Runtime.h"
#include "lazy.h"
#include "set.h"
#include "functional.h"
#include <tuple>
#include <stdexcept>
#include <utility>

namespace lightdb {
    class PhysicalLightField;
    using PhysicalLightFieldReference = shared_reference<PhysicalLightField, AddressableMixin<PhysicalLightField>>;

    class PhysicalLightField {
    public:
        inline std::string type() const noexcept { return typeid(*this).name(); }
        inline const LightFieldReference& logical() const noexcept { return logical_; }
        inline physical::DeviceType device() const noexcept { return deviceType_; }
        inline std::vector<PhysicalLightFieldReference>& parents() noexcept { return parents_; }
        inline const std::vector<PhysicalLightFieldReference>& parents() const noexcept { return parents_; }
        inline runtime::RuntimeReference runtime() { return runtime_; }

        virtual ~PhysicalLightField() = default;

    protected:
        PhysicalLightField(const LightFieldReference &logical,
                           const physical::DeviceType deviceType,
                           const lazy<runtime::RuntimeReference> &runtime)
                : PhysicalLightField(
                        logical, std::vector<PhysicalLightFieldReference>{}, deviceType, runtime)
        { }
        PhysicalLightField(const LightFieldReference &logical,
                           std::vector<PhysicalLightFieldReference> parents,
                           const physical::DeviceType deviceType,
                           lazy<runtime::RuntimeReference> runtime)
                : parents_(std::move(parents)),
                  logical_(logical),
                  deviceType_(deviceType),
                  runtime_(std::move(runtime))
        { }

    private:
        //TODO this should be a set
        std::vector<PhysicalLightFieldReference> parents_;
        const LightFieldReference logical_;
        const physical::DeviceType deviceType_;
        lazy<runtime::RuntimeReference> runtime_;
    };

    class FrameLightField: public PhysicalLightField {
    public:
        //inline const Configuration &configuration2() noexcept { return configuration_.value(); }

        template<typename Physical>
        class Runtime: public runtime::Runtime<Physical> {
        protected:
            explicit Runtime(Physical &physical)
                : runtime::Runtime<Physical>(physical)
            { }

        private:
//            const Configuration configuration_;
        };

    protected:
        explicit FrameLightField(const LightFieldReference &logical,
                                 const physical::DeviceType deviceType,
                                 const lazy<runtime::RuntimeReference> &runtime)
                                 //const Configuration &configuration)
                : FrameLightField(logical, std::vector<PhysicalLightFieldReference>{}, deviceType, runtime)
                                  //lazy<Configuration>{[configuration]() { return configuration; }})
        { }
        /*explicit FrameLightField(const LightFieldReference &logical, const physical::DeviceType deviceType,
                                 const lazy<runtime::RuntimeReference> &runtime)
                                 //const lazy<Configuration> &configuration)
                : FrameLightField(logical, std::vector<PhysicalLightFieldReference>{}, deviceType, runtime) //, configuration)
        { }*/
        explicit FrameLightField(const LightFieldReference &logical,
                                   const std::vector<PhysicalLightFieldReference> &parents,
                                   const physical::DeviceType deviceType,
                                   const lazy<runtime::RuntimeReference> &runtime)
                                   //lazy<Configuration> configuration)
                : PhysicalLightField(logical, parents, deviceType, runtime)
                  //configuration_(std::move(configuration))
        { }

    private:
        //lazy<Configuration> configuration_;
    };
} // namespace lightdb

#endif //LIGHTDB_PHYSICALOPERATORS_H
