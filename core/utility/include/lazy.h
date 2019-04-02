#ifndef LIGHTDB_MEMOIZE_H
#define LIGHTDB_MEMOIZE_H

#include "reference.h"
#include <optional>
#include <functional>

namespace lightdb {

template<typename T>
class lazy {
public:
    explicit lazy(const std::function<T()> &initializer) noexcept
            : initializer_(initializer), value_(nullptr)
    { }

    lazy(const lazy&) = default;
    lazy(lazy &&) noexcept = default;
    lazy& operator=(const lazy&) = default;
    lazy& operator=(lazy&&) noexcept = default;

    constexpr T* operator->() { return &value(); }
    constexpr T& operator*() & { return value(); }
    constexpr T&& operator*() &&  { return *value(); }

    constexpr operator T&() { return value(); }

    constexpr bool initialized() const noexcept { return static_cast<bool>(value_); }

    T& value() {
        if(value_ == nullptr)
            value_ = std::make_shared<T>(initializer_());
        return *value_;
    }

private:
    std::function<T()> initializer_;
    std::shared_ptr<T> value_;
};

} // namespace lightdb

#endif //LIGHTDB_MEMOIZE_H
