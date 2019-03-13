#ifndef LIGHTDBG_GPUOPERATORS_H
#define LIGHTDBG_GPUOPERATORS_H

#include "PhysicalOperators.h"

namespace lightdb::physical {
    class GPUOperator: public FrameLightField {
    public:
        inline const execution::GPU& gpu() const { return gpu_; }
        //inline const Configuration& configuration() { return output_configuration_; }

        template<typename Physical=GPUOperator>
        class Runtime: public FrameLightField::Runtime<Physical> {
        public:
            explicit Runtime(Physical &physical) //, Configuration configuration)
                : FrameLightField::Runtime<Physical>(physical),
                  context_(physical.gpu().context()),
                  lock_(context_)
                  //context_(physical.context_),
                  //lock_(physical.lock_)
                  //output_configuration_(configuration)
            { }

//            const Configuration &configuration() const { return configuration; }
            GPUContext& context() { return context_; }
            VideoLock& lock() {return lock_; }

        private:
            GPUContext context_;  //TODO remove ref
            VideoLock lock_; //TODO remove ref
            //Configuration output_configuration_;
        };

    protected:
        /*GPUOperator(const LightFieldReference &logical,
                    const std::vector<PhysicalLightFieldReference> &parents,
                    const lazy<runtime::RuntimeReference> &runtime,
                    const execution::GPU &gpu)
                    //const std::function<Configuration()> &output_configuration)
                : GPUOperator(logical, parents, runtime, gpu) //, lazy(output_configuration))
        { }*/

        GPUOperator(const LightFieldReference &logical,
                    const std::vector<PhysicalLightFieldReference> &parents,
                    const lazy<runtime::RuntimeReference> &runtime,
                    execution::GPU gpu)
                    //lazy<Configuration> output_configuration)
                : FrameLightField(logical, parents, DeviceType::GPU, runtime), //, output_configuration),
                  gpu_(gpu)
                  //context_([this]() { return this->gpu().context(); }),
                  //lock_([this]() { return VideoLock(context_); }),
                  //output_configuration_(std::move(output_configuration))
        { }

        GPUOperator(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent,
                    const lazy<runtime::RuntimeReference> &runtime)
                : GPUOperator(logical, {parent}, runtime,
                              parent.expect_downcast<GPUOperator>().gpu())
                              //[this]() { return parents().front().expect_downcast<GPUOperator>().configuration2(); })
        { }

/*        GPUOperator(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent,
                    const lazy<runtime::RuntimeReference> &runtime)
                    //const std::function<Configuration()> &output_configuration)
                : GPUOperator(logical, {parent}, runtime, parent.expect_downcast<GPUOperator>().gpu()) //, output_configuration)
        { }*/

        GPUOperator(const LightFieldReference &logical,
                    PhysicalLightFieldReference &parent,
                    const lazy<runtime::RuntimeReference> &runtime,
                    const execution::GPU &gpu)
                    //const std::function<Configuration()> &output_configuration)
                : GPUOperator(logical, std::vector<PhysicalLightFieldReference>{parent}, runtime, gpu) //, lazy<Configuration>{output_configuration})
        { }

        //TODO remove
        //GPUContext& context() { return context_; }
        //VideoLock& lock() {return lock_; }

    private:
        execution::GPU gpu_;
        //TODO move these to runtime
        //lazy<GPUContext> context_;
        //lazy<VideoLock> lock_;
        //lazy<Configuration> output_configuration_;
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
                             //const std::function<Configuration()> &output_configuration)
                : GPUOperator(logical, {parent}, runtime, gpu) //, lazy(output_configuration))
        { }

        /*explicit GPUUnaryOperator(const LightFieldReference &logical,
                                  PhysicalLightFieldReference &parent,
                                  const lazy<runtime::RuntimeReference> &runtime)
                                  //const std::function<Configuration()> &output_configuration)
                : GPUOperator(logical, parent, runtime) //, output_configuration)
        { }*/

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
        //virtual const Configuration &configuration() = 0;
    };
} // namespace lightdb::physical

#endif //LIGHTDBG_GPUOPERATORS_H
