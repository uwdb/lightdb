#ifndef LIGHTDB_REFERENCE_H
#define LIGHTDB_REFERENCE_H

#include <memory>
#include <optional>

namespace lightdb {

class _EmptyMixin { };

template<typename T, typename Mixin=_EmptyMixin>
class shared_reference: public Mixin {
public:
    explicit shared_reference(std::shared_ptr<T> pointer)
        : pointer_(std::move(pointer))
    { }

    shared_reference(const shared_reference &reference)
        : pointer_(reference.pointer_)
    { }

    template<typename TDerived>
    shared_reference(const TDerived &value)
        : pointer_{static_cast<std::shared_ptr<T>>(std::make_shared<TDerived>(value))}
    { }

    explicit inline operator T&() const {
        return *pointer_;
    }

    inline T* operator->() const {
        return pointer_.get();
    }

    inline T& operator*() const {
        return *pointer_;
    }

    explicit operator const std::shared_ptr<T>() const {
        return pointer_;
    }

    bool operator==(const shared_reference& other) const { return pointer_ == other.pointer_; }
    bool operator!=(const shared_reference& other) const { return !(*this == other); }

    template<typename TDerived>
    inline const TDerived& downcast() const {
        return *dynamic_cast<TDerived*>(pointer_.get());
    }
    template<typename TDerived>
    inline TDerived& downcast() {
        return *dynamic_cast<TDerived*>(pointer_.get());
    }

    template<typename TDerived>
    inline bool is() const {
        return dynamic_cast<const TDerived*>(pointer_.get()) != nullptr;
    }

    template<typename TDerived>
    inline const std::optional<std::reference_wrapper<const TDerived>> try_downcast() const {
        auto cast = dynamic_cast<const TDerived*>(pointer_.get());
        return cast != nullptr
            ? std::optional<std::reference_wrapper<const TDerived>>{*cast}
            : std::nullopt;
    }
    template<typename TDerived>
    inline std::optional<std::reference_wrapper<TDerived>> try_downcast() {
        auto cast = dynamic_cast<TDerived*>(pointer_.get());
        return cast != nullptr
               ? std::optional<std::reference_wrapper<TDerived>>{*cast}
               : std::nullopt;
    }

    inline std::string type() const {
        return typeid(*pointer_.get()).name();
    }

    template<typename TDerived, typename... _Args>
    inline static shared_reference
    make(_Args&&... args) {
        return shared_reference{static_cast<std::shared_ptr<T>>(std::make_shared<TDerived>(args...))};
    }

private:
    std::shared_ptr<T> pointer_;
};

} // namespace lightdb

#endif //LIGHTDB_REFERENCE_H
