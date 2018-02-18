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
        //: pointer_(std::static_cast<std::shared_ptr<T>>(std::make_shared<TDerived>(value)))
        : pointer_{static_cast<std::shared_ptr<T>>(std::make_shared<TDerived>(value))}
    { }

public:
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

    template<typename TDerived>
    inline const TDerived& downcast() const {
        return dynamic_cast<const TDerived&>(*pointer_.get());
    }

    template<typename TDerived>
    inline bool is() const {
        return dynamic_cast<const TDerived*>(pointer_.get()) != nullptr;
    }

    template<typename TDerived>
    inline const std::optional<std::reference_wrapper<const TDerived>> try_downcast() const {
        return {dynamic_cast<const TDerived&>(*this)};
    }

    template<typename TDerived, typename... _Args>
    inline static shared_reference
    make(_Args&&... args) {
        return shared_reference{static_cast<std::shared_ptr<T>>(std::make_shared<TDerived>(args...))};
    }

private:
    const std::shared_ptr<T> pointer_;
};

} // namespace lightdb

#endif //LIGHTDB_REFERENCE_H
