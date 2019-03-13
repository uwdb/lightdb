#ifndef LIGHTDBG_GPUOPERATORS_H
#define LIGHTDBG_GPUOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {
    class GPUOperator: public FrameLightField {
    public:
        inline const execution::GPU& gpu() const { return gpu_; }

        template<typename Physical=GPUOperator>
        class Runtime: public FrameLightField::Runtime<Physical> {
        public:
            explicit Runtime(Physical &physical)
                : FrameLightField::Runtime<Physical>(physical),
                  context_(physical.gpu().context()),
                  lock_(context_)
            { }

            GPUContext& context() { return context_; }
            VideoLock& lock() {return lock_; }

        private:
            GPUContext context_;
            VideoLock lock_;
        };

    protected:
        GPUOperator(const LightFieldReference &logical,
                    const std::vector<PhysicalLightFieldReference> &parents,
                    const lazy<runtime::RuntimeReference> &runtime,
                    execution::GPU gpu)
                : FrameLightField(logical, parents, DeviceType::GPU, runtime),
                  gpu_(gpu)
        { }

        GPUOperator(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent,
                    const lazy<runtime::RuntimeReference> &runtime)
                : GPUOperator(logical, {parent}, runtime,
                              parent.expect_downcast<GPUOperator>().gpu())
        { }

        GPUOperator(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent,
                    const lazy<runtime::RuntimeReference> &runtime,
                    const execution::GPU &gpu)
                : GPUOperator(logical, std::vector<PhysicalLightFieldReference>{parent}, runtime, gpu)
        { }

    private:
        execution::GPU gpu_;
    };

    class GPUUnaryOperator : public GPUOperator {
    public:
        template<typename T=PhysicalLightField>
        T& parent() noexcept { return functional::single(parents()).downcast<T>(); }

    protected:
        explicit GPUUnaryOperator(const LightFieldReference &logical,
                                  PhysicalLightFieldReference &parent,
                                  const lazy<runtime::RuntimeReference> &runtime)
                : GPUOperator(logical, {parent}, runtime)
        { }

        explicit GPUUnaryOperator(const LightFieldReference &logical,
                             const PhysicalLightFieldReference &parent,
                             const lazy<runtime::RuntimeReference> &runtime,
                             const execution::GPU &gpu)
                : GPUOperator(logical, {parent}, runtime, gpu)
        { }

        template<typename Physical, typename Data>
        class Runtime: public GPUOperator::Runtime<Physical> {
        public:
            explicit Runtime(Physical& physical)
                : GPUOperator::Runtime<Physical>(physical),
                  iterator_(this->iterators().front().template downcast<Data>()) {
                CHECK_EQ(physical.parents().size(), 1);
            }

            runtime::Runtime<>::downcast_iterator<Data>& iterator() noexcept { return iterator_; }

            template <typename T=Data, typename = typename std::enable_if<std::is_base_of<FrameData, T>::value>>
            Configuration configuration() { return (*iterator()).configuration(); }

        private:
            runtime::Runtime<>::downcast_iterator<Data> iterator_;
        };
    };

    class EncodedVideoInterface {
    public:
        virtual ~EncodedVideoInterface() = default;

        virtual const Codec &codec() const = 0;
    };
} // namespace lightdb::physical

#endif //LIGHTDBG_GPUOPERATORS_H
