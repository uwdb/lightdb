#ifndef LIGHTDB_ASSERT_H
#define LIGHTDB_ASSERT_H

#include <vector>
#include <stdexcept>

namespace lightdb::asserts {
    template<typename T>
    std::vector<T> &CHECK_NONEMPTY(std::vector<T> &volumes) {
        if (volumes.empty())
            throw std::runtime_error("empty"); //TODO
        else
            return volumes;
    }

    template<typename T>
    std::vector<T> &CHECK_NONOVERLAPPING(std::vector<T> &volumes) {
        return volumes; //TODO not implemented...
    }
} // namespace lightdb::assert

#endif //LIGHTDB_ASSERT_H
