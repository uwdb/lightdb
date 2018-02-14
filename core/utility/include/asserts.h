#ifndef LIGHTDB_ASSERT_H
#define LIGHTDB_ASSERT_H

#include "errors.h"
#include <vector>
#include <stdexcept>

namespace lightdb::asserts {
    template<typename T>
    std::vector<T> &CHECK_NONEMPTY(std::vector<T> &volumes) {
        if (volumes.empty())
            throw InvalidArgumentError("Expected nonempty vector", "volumes");
        else
            return volumes;
    }

    template<typename T>
    std::vector<T> &CHECK_NONOVERLAPPING(std::vector<T> &volumes) {
        return volumes; //TODO not implemented...
    }
} // namespace lightdb::assert

#endif //LIGHTDB_ASSERT_H
