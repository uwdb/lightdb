#ifndef LIGHTDB_REFERENCE_H
#define LIGHTDB_REFERENCE_H

#include "errors.h"
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
          #ifndef NDEBUG
            , direct_(&*pointer_)
          #endif
    { Mixin::PostConstruct(*this); }

    shared_reference(const shared_reference &reference)
        : Mixin(*this), pointer_(reference.pointer_)
          #ifndef NDEBUG
            , direct_(&*pointer_)
          #endif
    { Mixin::PostConstruct(*this); }

    template<typename TDerived>
    shared_reference(const std::shared_ptr<TDerived> &value)
            : Mixin(*this), pointer_{std::dynamic_pointer_cast<T>(value)}
              #ifndef NDEBUG
                , direct_(&*pointer_)
              #endif
    { Mixin::PostConstruct(*this); }

    template<typename TDerived>
    shared_reference(const shared_reference<TDerived> &value)
            : Mixin(*this),
              pointer_{std::dynamic_pointer_cast<T>(static_cast<std::shared_ptr<TDerived>>(value))}
    #ifndef NDEBUG
                , direct_(&*pointer_)
    #endif
    { Mixin::PostConstruct(*this); }

    shared_reference(const T &value)
            : Mixin(*this), pointer_{std::make_shared<T>(value)}
            #ifndef NDEBUG
                , direct_(&*pointer_)
            #endif
    {
        Mixin::PostConstruct(*this);
    }

    template<typename TDerived>
    shared_reference(const TDerived &value, typename std::enable_if<std::is_base_of<T,TDerived>::value>::type* = 0)
        : Mixin(*this), pointer_{std::make_shared<TDerived>(value)}
          #ifndef NDEBUG
            , direct_(&*pointer_)
          #endif
    {
        Mixin::PostConstruct(*this);
    }

    template<typename TDerived>
    shared_reference(TDerived &value, typename std::enable_if<std::is_base_of<T,TDerived>::value>::type* = 0)
            : Mixin(*this), pointer_{std::make_shared<TDerived>(value)}
              #ifndef NDEBUG
                , direct_(&*pointer_)
              #endif
    {
        Mixin::PostConstruct(*this);
    }

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
    bool operator<(const shared_reference& other) const { return pointer_ < other.pointer_; }
    bool operator<=(const shared_reference& other) const { return pointer_ <= other.pointer_; }
    bool operator>(const shared_reference& other) const { return pointer_ > other.pointer_; }
    bool operator>=(const shared_reference& other) const { return pointer_ >= other.pointer_; }

    shared_reference& operator=(const shared_reference&) = default;
    shared_reference& operator=(shared_reference&&) noexcept = default;

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
    #ifndef NDEBUG
    T* direct_;
    #endif
};

template<typename T>
class AddressableMixin : public std::enable_shared_from_this<T> {
public:
    void PostConstruct(const shared_reference<T, AddressableMixin>& instance) { }

    shared_reference<T, AddressableMixin> get() {
        return shared_reference<T, AddressableMixin>(this->shared_from_this());
    };
};

} // namespace lightdb

#endif //LIGHTDB_REFERENCE_H
