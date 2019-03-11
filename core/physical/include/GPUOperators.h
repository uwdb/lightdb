#ifndef LIGHTDBG_GPUOPERATORS_H
#define LIGHTDBG_GPUOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {
    class GPUOperator: public FrameLightField {
    public:
        inline const execution::GPU& gpu() const { return gpu_; }
        //inline const Configuration& configuration() { return output_configuration_; }

        GPUContext& context() { return context_; }
        VideoLock& lock() {return lock_; }

    protected:
        GPUOperator(const LightFieldReference &logical,
                    const std::vector<PhysicalLightFieldReference> &parents,
                    const execution::GPU &gpu,
                    const std::function<Configuration()> &output_configuration)
                : GPUOperator(logical, parents, gpu, lazy(output_configuration))
        { }

        GPUOperator(const LightFieldReference &logical,
                    const std::vector<PhysicalLightFieldReference> &parents,
                    execution::GPU gpu,
                    lazy<Configuration> output_configuration)
                : FrameLightField(logical, parents, DeviceType::GPU, output_configuration),
                  gpu_(gpu),
                  context_([this]() { return this->gpu().context(); }),
                  lock_([this]() { return VideoLock(context()); }),
                  output_configuration_(std::move(output_configuration))
        { }

        GPUOperator(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent)
                : GPUOperator(logical, {parent},
                              parent.expect_downcast<GPUOperator>().gpu(),
                              [this]() { return parents().front().expect_downcast<GPUOperator>().configuration2(); })
        { }

        GPUOperator(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent,
                    const std::function<Configuration()> &output_configuration)
                : GPUOperator(logical, {parent}, parent.expect_downcast<GPUOperator>().gpu(), output_configuration)
        { }

        GPUOperator(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent,
                    const execution::GPU &gpu,
                    const std::function<Configuration()> &output_configuration)
                : GPUOperator(logical, {parent}, gpu, lazy<Configuration>{output_configuration})
        { }

    private:
        execution::GPU gpu_;
        lazy<GPUContext> context_;
        lazy<VideoLock> lock_;
        lazy<Configuration> output_configuration_;
    };

    template<typename Data>
    class GPUUnaryOperator : public GPUOperator {
    public:
        template<typename T=PhysicalLightField>
        T& parent() noexcept { return parents().front().downcast<T>(); }
        downcast_iterator<Data>& iterator() noexcept { return iterator_; }

    protected:
        explicit GPUUnaryOperator(const LightFieldReference &logical,
                                  PhysicalLightFieldReference &parent)
                : GPUOperator(logical, {parent}),
                  iterator_([this]() { return iterators().front().downcast<Data>(); })
        { }

        explicit GPUUnaryOperator(const LightFieldReference &logical,
                             const PhysicalLightFieldReference &parent,
                             const execution::GPU &gpu,
                             const std::function<Configuration()> &output_configuration)
                : GPUOperator(logical, {parent}, gpu, lazy(output_configuration)),
                  iterator_([this]() { return iterators().front().downcast<Data>(); })
        { }

        explicit GPUUnaryOperator(const LightFieldReference &logical,
                                  PhysicalLightFieldReference &parent,
                                  const std::function<Configuration()> &output_configuration)
                : GPUOperator(logical, parent, output_configuration),
                  iterator_([this]() { return iterators().front().downcast<Data>(); })
        { }

    private:
        lazy<PhysicalLightField::downcast_iterator<Data>> iterator_;
    };

    class EncodedVideoInterface {
    public:
        virtual ~EncodedVideoInterface() = default;

        virtual const Codec &codec() const = 0;
        virtual const Configuration &configuration() = 0;
    };
} // namespace lightdb::physical

#endif //LIGHTDBG_GPUOPERATORS_H
