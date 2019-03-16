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
    class PhysicalOperator;
    using PhysicalOperatorReference = shared_reference<PhysicalOperator, AddressableMixin<PhysicalOperator>>;

    class PhysicalOperator {
    public:
        inline std::string type() const noexcept { return typeid(*this).name(); }
        inline const LightFieldReference& logical() const noexcept { return logical_; }
        inline physical::DeviceType device() const noexcept { return deviceType_; }
        inline std::vector<PhysicalOperatorReference>& parents() noexcept { return parents_; }
        inline const std::vector<PhysicalOperatorReference>& parents() const noexcept { return parents_; }
        inline runtime::RuntimeReference runtime() { return runtime_; }

        virtual ~PhysicalOperator() = default;

    protected:
        PhysicalOperator(const LightFieldReference &logical,
                           const physical::DeviceType deviceType,
                           const lazy<runtime::RuntimeReference> &runtime)
                : PhysicalOperator(
                        logical, std::vector<PhysicalOperatorReference>{}, deviceType, runtime)
        { }
        PhysicalOperator(const LightFieldReference &logical,
                           std::vector<PhysicalOperatorReference> parents,
                           const physical::DeviceType deviceType,
                           lazy<runtime::RuntimeReference> runtime)
                : parents_(std::move(parents)),
                  logical_(logical),
                  deviceType_(deviceType),
                  runtime_(std::move(runtime))
        { }

    private:
        //TODO this should be a set
        std::vector<PhysicalOperatorReference> parents_;
        const LightFieldReference logical_;
        const physical::DeviceType deviceType_;
        lazy<runtime::RuntimeReference> runtime_;
    };

    namespace physical {
        class GPUOperator {
        public:
            inline const execution::GPU& gpu() const { return gpu_; }

        protected:
            explicit GPUOperator(PhysicalOperatorReference &parent)
                    : GPUOperator(parent.expect_downcast<GPUOperator>())
            { }

            explicit GPUOperator(const execution::GPU &gpu)
                    : gpu_(gpu)
            { }

            GPUOperator(const GPUOperator &parent) = default;

        private:
            const execution::GPU gpu_;
        };

        class UnaryOperator {
        protected:
            template<typename Physical=PhysicalOperator>
            Physical& parent() noexcept {
                return functional::single(reinterpret_cast<PhysicalOperator*>(this)->parents()).template downcast<Physical>();
            }
        };
    } // namespace physical
} // namespace lightdb

#endif //LIGHTDB_PHYSICALOPERATORS_H
