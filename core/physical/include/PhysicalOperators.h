#ifndef LIGHTDB_PHYSICALOPERATORS_H
#define LIGHTDB_PHYSICALOPERATORS_H

#include "LightField.h"
#include "Encoding.h"
#include "Environment.h"
#include "MaterializedLightField.h"
#include "lazy.h"
#include "functional.h"
#include <tuple>
#include <stdexcept>
#include <utility>

namespace lightdb {
    class PhysicalLightField;
    using PhysicalLightFieldReference = shared_reference<PhysicalLightField, AddressableMixin<PhysicalLightField>>;

    class PhysicalLightField {
    public:
        class iterator;

        inline std::string type() const noexcept { return typeid(*this).name(); }
        inline const LightFieldReference& logical() const noexcept { return logical_; }
        inline physical::DeviceType device() const noexcept { return deviceType_; }
        inline auto& parents() noexcept { return parents_; }

        template<typename T>
        class downcast_iterator;

        class iterator {
            friend class PhysicalLightField;

        public:
            static const iterator& eos() { return eos_instance_; }

            bool operator==(const iterator& other) const { return (eos_ && other.eos_) ||
                                                                  (physical_ == other.physical_ &&
                                                                   *current_ == *other.current_); }
            bool operator!=(const iterator& other) const { return !(*this == other); }
            void operator++() {
                assert(!eos_);
                current_.reset();
                eos_ = !(current_ = physical_->read()).has_value();
            }
            physical::MaterializedLightFieldReference operator++(int)
            {
                auto value = **this;
                ++*this;
                return value;
            }
            physical::MaterializedLightFieldReference operator*() { return current_.value(); }

            PhysicalLightField& physical() const { return *physical_; }

            template<typename T>
            downcast_iterator<T> downcast() { return downcast_iterator<T>(*this); }

        protected:
            explicit iterator(PhysicalLightField &physical)
                    : physical_(&physical), current_(physical.read()), eos_(!current_.has_value())
            { }
            constexpr explicit iterator()
                    : physical_(nullptr), current_(), eos_(true)
            { }

        private:
            static iterator eos_instance_;
            PhysicalLightField *physical_;
            std::optional<physical::MaterializedLightFieldReference> current_;
            bool eos_;
        };

        //TODO this should move to functional.h, all it does is downcast *this
        template<typename T>
        class downcast_iterator {
            friend class iterator;

        public:
            bool operator==(const downcast_iterator<T>& other) const { return iterator_ == other.iterator_; }
            bool operator!=(const downcast_iterator<T>& other) const { return !(*this == other); }
            void operator++() {
                assert(iterator_ != iterator::eos());
                ++iterator_;
            }
            T operator++(int)
            {
                auto value = **this;
                ++*this;
                return std::move(value);
            }
            T operator*() { return (*iterator_).expect_downcast<T>(); }

            static const downcast_iterator<T> eos() { return downcast_iterator<T>(); }

        protected:
            explicit downcast_iterator(iterator& iterator)
                    : iterator_(iterator)
            { }

            constexpr explicit downcast_iterator()
                    : iterator_(iterator::eos_instance_)
            { }

        private:
            iterator& iterator_;
        };

        virtual iterator begin() { return iterator(*this); }
        virtual const iterator& end() { return iterator::eos(); }
        virtual std::optional<physical::MaterializedLightFieldReference> read() = 0;

    protected:
        explicit PhysicalLightField(const LightFieldReference &logical, const physical::DeviceType deviceType)
                : PhysicalLightField(logical, std::vector<PhysicalLightFieldReference>{}, deviceType)
        { }
        explicit PhysicalLightField(const LightFieldReference &logical,
                                    std::vector<PhysicalLightFieldReference> parents,
                                    const physical::DeviceType deviceType)
                : parents_(std::move(parents)),
                  logical_(logical),
                  deviceType_(deviceType),
                  iterators_([this] () -> std::vector<iterator> {
                      return functional::transform<iterator>(parents_.begin(), parents_.end(), [](auto &parent) {
                             return parent->begin(); }); })
        { }

        virtual ~PhysicalLightField() = default;

        std::vector<iterator>& iterators() noexcept { return iterators_; }
        bool any_parent_eos() noexcept { return std::any_of(iterators_->begin(),
                                                            iterators_->end(),
                                                            [](auto &it) { return it == iterator::eos(); }); }
        bool all_parent_eos() noexcept { return std::all_of(iterators_->begin(),
                                                            iterators_->end(),
                                                            [](auto &it) { return it == iterator::eos(); }); }

    private:
        //TODO this should be a set
        std::vector<PhysicalLightFieldReference> parents_;
        const LightFieldReference logical_;
        const physical::DeviceType deviceType_;
        lazy<std::vector<iterator>> iterators_;
    };


    namespace physical {
        class GPUOperator: public PhysicalLightField {
        public:
            inline const execution::GPU& gpu() const { return gpu_; }
            inline const Configuration& configuration() { return output_configuration_; }

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
                        std::vector<PhysicalLightFieldReference> parents,
                        execution::GPU gpu,
                        lazy<Configuration> output_configuration)
                    : PhysicalLightField(logical, std::move(parents), DeviceType::GPU),
                      gpu_(gpu),
                      context_([this]() { return this->gpu().context(); }),
                      lock_([this]() { return VideoLock(context()); }),
                      output_configuration_(std::move(output_configuration))
            { }

            GPUOperator(const LightFieldReference &logical,
                        PhysicalLightFieldReference &parent)
                    : GPUOperator(logical, {parent},
                                  parent.expect_downcast<GPUOperator>().gpu(),
                                  [this]() { return parents().front().expect_downcast<GPUOperator>().configuration(); })
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
    } // namespace physical
} // namespace lightdb

#endif //LIGHTDB_PHYSICALOPERATORS_H
