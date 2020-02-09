#ifndef LIGHTDB_ASSERT_H
#define LIGHTDB_ASSERT_H

#include "errors.h"
#include <vector>
#include <stdexcept>
#include <filesystem>

namespace lightdb::asserts {
    template<typename T>
    std::vector<T> &CHECK_NONEMPTY(std::vector<T> &volumes) {
        if (volumes.empty())
            throw InvalidArgumentError("Expected nonempty vector", "volumes");
        else
            return volumes;
    }

    template<typename T>
    const std::vector<T> &CHECK_NONEMPTY(const std::vector<T> &volumes) {
        if (volumes.empty())
            throw InvalidArgumentError("Expected nonempty vector", "volumes");
        else
            return std::move(volumes);
    }

    inline std::string& CHECK_NONEMPTY(std::string &value) {
        if (value.empty())
            throw InvalidArgumentError("Expected nonempty value", "value");
        else
            return value;
    }

    inline std::filesystem::path& CHECK_NONEMPTY(std::filesystem::path& path) {
        if (path.empty())
            throw InvalidArgumentError("Expected nonempty value", "value");
        else
            return path;
    }

    template<typename T>
    std::vector<T> &CHECK_NONOVERLAPPING(std::vector<T> &volumes) {
        LOG(WARNING) << "Non-overlapping check not implemented"; //TODO
        return volumes;
    }

    template<typename T>
    const std::vector<T> &CHECK_NONOVERLAPPING(const std::vector<T> &volumes) {
        LOG(WARNING) << "Non-overlapping check not implemented"; //TODO
        return volumes;
    }

    template<typename T>
    const T &CHECK_POSITIVE(const T &value) {
        if(value > 0)
            return value;
        else
            throw InvalidArgumentError("Expected positive value", "value");
    }
} // namespace lightdb::assert

#endif //LIGHTDB_ASSERT_H
