#ifndef LIGHTDB_FUNCTOR_H
#define LIGHTDB_FUNCTOR_H

#include "LightField.h"
#include "MaterializedLightField.h"
#include "functional.h"
#include <map>

namespace lightdb {
    class PhysicalLightField;

    namespace functor {
        namespace internal {
            template<typename, typename>
            class _nary_builder;

            template<typename T, std::size_t... S>
            class _nary_builder<T, std::index_sequence<S...>> {
                template<std::size_t>
                using type = T;

            public:
                virtual shared_reference<typename std::remove_reference<T>::type> operator()(const type<S>...) = 0;
            };
        } // namespace internal


        template<std::size_t n>
        class naryfunction: public functor::internal::_nary_builder<LightField&, std::make_index_sequence<n>> {
        public:
            explicit naryfunction(const physical::DeviceType deviceType, Codec codec)
                    : deviceType_(deviceType),
                      codec_(std::move(codec))
            { }

            virtual ~naryfunction() = default;

            const Codec& codec() const { return codec_; }
            physical::DeviceType device() const { return deviceType_; }

        private:
            const physical::DeviceType deviceType_;
            const Codec codec_;
        };

        template<std::size_t n>
        using NaryFunctionReference = shared_reference<naryfunction<n>>;
        using UnaryFunctionReference = NaryFunctionReference<1>;


        template<std::size_t n>
        class naryfunctor {
        public:
            template<typename... Args>
            explicit naryfunctor(const std::string &name, const Args&... functions)
                    : naryfunctor(name, std::initializer_list<NaryFunctionReference<n>>{functions...})
            { }

            virtual ~naryfunctor() = default;

            const std::string& name() const noexcept { return name_; }
            naryfunction<n>& operator()(const physical::DeviceType type) const {
                return *implementations_.at(type);
            }
            bool has_implementation(const physical::DeviceType type) const {
                return implementations_.find(type) != implementations_.end();
            }
            const naryfunction<n>& preferred_implementation() const {
                if(has_implementation(physical::DeviceType::GPU))
                    return (*this)(physical::DeviceType::GPU);
                else if(has_implementation(physical::DeviceType::FPGA))
                    return (*this)(physical::DeviceType::FPGA);
                else if(has_implementation(physical::DeviceType::CPU))
                    return (*this)(physical::DeviceType::CPU);
                else
                    throw NotImplementedError("Functor has no implementations");
            }

        private:
            explicit naryfunctor(std::string name, const std::initializer_list<NaryFunctionReference<n>> &functions)
                    : naryfunctor(name,
                                  functional::transform<std::pair<physical::DeviceType, NaryFunctionReference<n>>>(
                                          functions.begin(), functions.end(), [](auto &f) {
                                              return std::make_pair(f->device(), f); }))
            { }

            explicit naryfunctor(std::string name, const std::vector<std::pair<physical::DeviceType, NaryFunctionReference<n>>> &functions)
                    : name_(std::move(name)),
                      implementations_(functions.begin(), functions.end())
            { }

            const std::string name_;
            const std::map<physical::DeviceType, NaryFunctionReference<n>> implementations_;
        };

        template<std::size_t n>
        using FunctorReference = shared_reference<naryfunctor<n>>;
        using UnaryFunctorReference = FunctorReference<1>;
        using BinaryFunctorReference = FunctorReference<2>;

        using unaryfunctor = naryfunctor<1>;
        using binaryfunctor = naryfunctor<2>;
        using unaryfunction = naryfunction<1>;
        using binaryfunction = naryfunction<2>;
    }; // namespace functor
}; // namespace lightdb

#endif //LIGHTDB_FUNCTOR_H
