#ifndef LIGHTDB_ERRORS_H
#define LIGHTDB_ERRORS_H

#include <vector>
#include <stdexcept>

namespace lightdb::errors {
    template<typename T>
    std::vector<T>& CHECK_NONEMPTY(std::vector<T>& volumes)
    {
        if(volumes.empty())
            throw std::runtime_error("empty"); //TODO
        else
            return volumes;
    }
}

#endif //LIGHTDB_ERRORS_H
