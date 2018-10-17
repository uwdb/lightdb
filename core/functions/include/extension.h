#ifndef LIGHTDB_EXTENSION_H
#define LIGHTDB_EXTENSION_H

#include "Functor.h"
#include <experimental/filesystem>

namespace lightdb::extensibility {

std::shared_ptr<functor::unaryfunctor> Load(const std::string &name,
                                            const std::experimental::filesystem::path &path="plugins");

}; // namespace lightdb

#endif //LIGHTDB_EXTENSION_H
