#ifndef LIGHTDB_REFERENCE_H
#define LIGHTDB_REFERENCE_H

#include <memory>
#include <unordered_map>
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

    virtual ~shared_reference() = default;

    explicit inline operator T&() const {
        return *pointer_;
    }

    inline T* operator->() const {
        return pointer_.get();
    }

    inline T& operator*() const {
        return *pointer_;
    }

    explicit operator const std::shared_ptr<T>&() const {
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
    inline const TDerived& expect_downcast() const {
        if(is<TDerived>())
            return downcast<TDerived>();
        else
            throw BadCastError("Could not downcast instance of type " + type() + " to " + typeid(TDerived).name());
    }
    template<typename TDerived>
    inline TDerived& expect_downcast() {
        if(is<TDerived>())
            return downcast<TDerived>();
        else
            throw BadCastError("Could not downcast instance of type " + type() + " to " + typeid(TDerived).name());
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

template<typename T, typename Mixin=_EmptyMixin>
class shared_addressable_reference: public shared_reference<T, Mixin> {
public:
    explicit shared_addressable_reference(const std::shared_ptr<T>& pointer)
            : shared_reference<T, Mixin>(pointer) {
        Register(*this);
    }

    shared_addressable_reference(const shared_addressable_reference &reference)
            : shared_reference<T, Mixin>(static_cast<const shared_reference<T, Mixin>&>(reference)) {
        Register(*this);
    }

    template<typename TDerived>
    shared_addressable_reference(const TDerived &value)
            : shared_reference<T, Mixin>(value) {
        Register(*this);
    }

    virtual ~shared_addressable_reference() {
        Unregister(*this);
    }

    static shared_addressable_reference<T, Mixin> get(const T& instance) {
        auto weak = lookup_[&instance];
        auto shared = weak.lock();
        return shared_addressable_reference<T, Mixin>(shared);
    };

private:
    static void Register(const shared_addressable_reference<T, Mixin>& instance) {
        Register(static_cast<const std::shared_ptr<T>&>(
                         static_cast<const shared_reference<T, Mixin>&>(instance)));
    }
    static void Register(const std::shared_ptr<T>& pointer) {
        if(pointer != nullptr)
            lookup_.emplace(std::make_pair(&*pointer, std::weak_ptr<T>(pointer)));
    }

    static void Unregister(const shared_addressable_reference<T, Mixin>& instance) {
        Unregister(static_cast<const std::shared_ptr<T>&>(
                           static_cast<const shared_reference<T, Mixin>&>(instance)));
    }
    static void Unregister(const std::shared_ptr<T>& pointer) {
        if(pointer != nullptr && pointer.unique())
            lookup_.erase(&*pointer);
    }

    static std::unordered_map<T*, std::weak_ptr<T>> lookup_;
};

template<typename T, typename Mixin>
std::unordered_map<T*, std::weak_ptr<T>> shared_addressable_reference<T, Mixin>::lookup_{};

} // namespace lightdb

#endif //LIGHTDB_REFERENCE_H
