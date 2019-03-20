#ifndef LIGHTDB_RUNTIME_H
#define LIGHTDB_RUNTIME_H

#include "MaterializedLightField.h"
#include "functional.h"

namespace lightdb {
    class PhysicalOperator;

    namespace runtime {
        template<typename... T>
        class Runtime;

        using RuntimeReference = shared_reference<Runtime<>>;

        template<>
        class Runtime<> {
        public:
            Runtime(const Runtime&) = delete;
            Runtime(Runtime&&) = default;

            class iterator;
            template<typename T>
            using downcast_iterator = functional::downcast_iterator<T, iterator>;

            class iterator {
                friend class Runtime;

            public:
                static iterator &eos() { return eos_instance_; }

                bool operator==(const iterator &other) const {
                    return (eos_ && other.eos_) ||
                           (runtime_ == other.runtime_ &&
                            *current_ == *other.current_);
                }

                bool operator!=(const iterator &other) const { return !(*this == other); }

                void operator++() {
                    assert(!eos_);
                    current_.reset();
                    eos_ = !(current_ = runtime_->read()).has_value();
                }

                const physical::MaterializedLightFieldReference operator++(int) {
                    auto value = **this;
                    ++*this;
                    return value;
                }

                physical::MaterializedLightFieldReference operator*() { return current_.value(); }

                Runtime &runtime() const { return *runtime_; }

                template<typename T>
                downcast_iterator<T> downcast() { return downcast_iterator<T>(*this); }

            protected:
                explicit iterator(Runtime &runtime)
                        : runtime_(&runtime), current_(runtime.read()), eos_(!current_.has_value()) {}

                constexpr explicit iterator() noexcept
                        : runtime_(nullptr), current_(), eos_(true) {}

            private:
                static iterator eos_instance_;
                Runtime *runtime_;
                std::optional<physical::MaterializedLightFieldReference> current_;
                bool eos_;
            };

            inline virtual iterator begin() { return iterator(*this); }

            inline virtual const iterator &end() { return iterator::eos(); }

            //TODO make this protected after GPUOperatorAdapter is removed
            virtual std::optional<physical::MaterializedLightFieldReference> read() = 0;
            std::vector<iterator> &iterators() noexcept { return iterators_; }

        protected:
            explicit Runtime(PhysicalOperator &physical);

            virtual ~Runtime() = default;

            inline virtual PhysicalOperator &physical() { return physical_; }
            inline virtual const PhysicalOperator &physical() const { return physical_; }

            LightFieldReference logical() const;

            bool any_parent_eos() noexcept {
                return std::any_of(iterators_.begin(),
                                   iterators_.end(),
                                   [](auto &it) { return it == iterator::eos(); });
            }

            bool all_parent_eos() noexcept {
                return std::all_of(iterators_.begin(),
                                   iterators_.end(),
                                   [](auto &it) { return it == iterator::eos(); });
            }

        private:
            PhysicalOperator &physical_;
            std::vector<iterator> iterators_;
        };

        template<typename Physical>
        class Runtime<Physical> : public Runtime<> {
        protected:
            inline Physical &physical() override { return physical_; }

            inline const Physical &physical() const override { return physical_; }

            explicit Runtime(Physical &physical)
                    : Runtime<>(physical),
                      physical_(physical) {}

        private:
            Physical &physical_;
        };

        template<typename Physical, typename Data, typename BaseRuntime=Runtime<Physical>>
        class UnaryRuntime: public BaseRuntime {
        protected:
            explicit UnaryRuntime(Physical &physical)
                    : BaseRuntime(physical),
                      iterator_(this->iterators().front().template downcast<Data>()) {
                CHECK_EQ(physical.parents().size(), 1);
            }

            template<typename T=PhysicalOperator>
            T& parent() noexcept { return functional::single(this->physical().parents()).template downcast<T>(); }

            runtime::Runtime<>::downcast_iterator<Data>& iterator() noexcept { return iterator_; }

            template <typename T=Data, typename = typename std::enable_if<std::is_base_of<physical::FrameData, T>::value>>
            Configuration configuration() { return (*iterator()).configuration(); }

            template <typename T=Data, typename = typename std::enable_if<std::is_base_of<physical::FrameData, T>::value>>
            GeometryReference geometry() { return (*iterator()).geometry(); }

            template <typename T=Data, typename = typename std::enable_if<std::is_base_of<physical::EncodedFrameData, T>::value>>
            Codec codec() { return (*iterator()).codec(); }

        private:
            runtime::Runtime<>::template downcast_iterator<Data> iterator_;
        };

        template<typename Physical=PhysicalOperator>
        class GPURuntime: public runtime::Runtime<Physical> {
        public:
            explicit GPURuntime(Physical &physical)
                    : runtime::Runtime<Physical>(physical),
                      context_(physical.gpu().context()),
                      lock_(context_)
            { }

            GPUContext& context() { return context_; }
            VideoLock& lock() {return lock_; }

        private:
            GPUContext context_;
            VideoLock lock_;
        };

        template<typename Physical, typename Data>
        using GPUUnaryRuntime = runtime::UnaryRuntime<Physical, Data, GPURuntime<Physical>>;

        template <typename Runtime,
                  typename Physical,
                  typename... Args,
                  typename = typename std::enable_if<(std::is_move_constructible<Args>::value && ...)>::type>
        static auto make(Physical& physical, Args&&... args) {
            return lazy<RuntimeReference>{[
                    &physical,
                    args = std::make_tuple(std::forward<Args>(args) ...)]() mutable {
                return std::apply([&physical](auto&& ... args) {
                    return RuntimeReference::make<Runtime>(physical, args...);
                    }, std::move(args));
                }
            };
        }

        template <typename Runtime,
                  typename Physical,
                  typename... Args,
                  typename = typename std::enable_if<!(std::is_move_constructible<Args>::value && ...)>::type>
        static auto make(Physical &physical, Args&... args) {
            return lazy<RuntimeReference>{[&physical, args...]() mutable {
                return RuntimeReference::make<Runtime>(physical, args...); } };
        }

        using RuntimeIterator = Runtime<>::iterator;
    } // namespace runtime
} //namespace lightdb

#endif //LIGHTDB_RUNTIME_H
