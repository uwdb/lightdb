#ifndef LIGHTDBG_GPUOPERATORS_H
#define LIGHTDBG_GPUOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {
    class GPUOperator {
    public:
        inline const execution::GPU& gpu() const { return gpu_; }

    protected:
        explicit GPUOperator(PhysicalLightFieldReference &parent)
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
        template<typename Physical=PhysicalLightField>
        Physical& parent() noexcept {
            return functional::single(reinterpret_cast<PhysicalLightField*>(this)->parents()).template downcast<Physical>();
        }
    };

    class EncodedVideoOperator {
    public:
        virtual ~EncodedVideoOperator() = default;

        virtual const Codec &codec() const = 0;
    };
} // namespace lightdb::physical

#endif //LIGHTDBG_GPUOPERATORS_H
