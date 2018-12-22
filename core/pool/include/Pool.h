#ifndef LIGHTDB_POOL_H
#define LIGHTDB_POOL_H

#include "MaterializedLightField.h"

namespace lightdb {

    namespace physical {
        class MaterializedLightField;
    } // namespace physical

    namespace pool {
        template<typename T>
        class BufferPoolEntry {
        public:
            virtual ~BufferPoolEntry() {
                Unregister(static_cast<shared_reference<T, BufferPoolEntry>&>(*this));
            }

            void PostConstruct(const shared_reference<T, BufferPoolEntry>& instance) {
                Register(static_cast<const std::shared_ptr<T>&>(instance));
            }

            static shared_reference<T, BufferPoolEntry> get(const T& instance) {
                auto weak = lookup_.at(&instance);
                auto shared = weak.lock();
                return shared_reference<T, BufferPoolEntry>(shared);
            };

            static size_t size() { return lookup_.size(); }

        private:
            static void Register(const std::shared_ptr<T>& pointer) {
                assert(pointer != nullptr);
                lookup_.emplace(std::make_pair(&*pointer, std::weak_ptr<T>(pointer)));
            }

            static void Unregister(const shared_reference<T, BufferPoolEntry>& instance) {
                Unregister(&*instance);
            }

            static void Unregister(const T* pointer) {
                if(pointer != nullptr && lookup_.at(pointer).use_count() == 0)
                    lookup_.erase(pointer);
            }

            static std::unordered_map<const T*, std::weak_ptr<T>> lookup_;
        };

        template<typename T>
        std::unordered_map<const T*, std::weak_ptr<T>> BufferPoolEntry<T>::lookup_{};
    } // namespace pool
} // namespace lightdb

#endif //LIGHTDB_POOL_H
