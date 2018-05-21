#ifndef LIGHTDB_REFERENCE_H
#define LIGHTDB_REFERENCE_H

#include <memory>
#include <unordered_map>
#include <optional>

namespace lightdb {

class DefaultMixin {
public:
    inline void PostConstruct(const DefaultMixin&) const { }
};

template<typename T, typename Mixin=DefaultMixin>
class shared_reference: public Mixin {
public:
    //TODO Change PostConstruct to use SFINAE
    explicit shared_reference(std::shared_ptr<T> pointer)
        : Mixin(*this),
          pointer_(std::move(pointer))
    { Mixin::PostConstruct(*this); }

    shared_reference(const shared_reference &reference)
        : Mixin(*this), pointer_(reference.pointer_)
    { Mixin::PostConstruct(*this); }

    template<typename TDerived>
    shared_reference(const std::shared_ptr<TDerived> &value)
            : Mixin(*this), pointer_{std::dynamic_pointer_cast<T>(value)}
    { Mixin::PostConstruct(*this); }

    template<typename TDerived>
    shared_reference(const TDerived &value)
        : Mixin(*this), pointer_{static_cast<std::shared_ptr<T>>(std::make_shared<TDerived>(value))}
    { Mixin::PostConstruct(*this); }

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

    inline explicit operator const std::shared_ptr<T>&() const {
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

template<typename T>
class AddressableMixin {
public:
    virtual ~AddressableMixin() {
        Unregister(static_cast<shared_reference<T, AddressableMixin>&>(*this));
    }

    void PostConstruct(const shared_reference<T, AddressableMixin>& instance) {
        Register(static_cast<const std::shared_ptr<T>&>(instance));
    }

    static shared_reference<T, AddressableMixin> get(const T& instance) {
        auto weak = lookup_[&instance];
        auto shared = weak.lock();
        return shared_reference<T, AddressableMixin>(shared);
    };

private:
    static void Register(const std::shared_ptr<T>& pointer) {
        if(pointer != nullptr)
            lookup_.emplace(std::make_pair(&*pointer, std::weak_ptr<T>(pointer)));
    }

    static void Unregister(const shared_reference<T, AddressableMixin>& instance) {
        Unregister(static_cast<const std::shared_ptr<T>&>(instance));
    }
    static void Unregister(const std::shared_ptr<T>& pointer) {
        if(pointer != nullptr && pointer.unique())
            lookup_.erase(&*pointer);
    }

    static std::unordered_map<T*, std::weak_ptr<T>> lookup_;
};

template<typename T>
std::unordered_map<T*, std::weak_ptr<T>> AddressableMixin<T>::lookup_{};

} // namespace lightdb

#endif //LIGHTDB_REFERENCE_H
