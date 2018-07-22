#ifndef LIGHTDB_EXTENSION_H
#define LIGHTDB_EXTENSION_H

#include "Functor.h"
#include <boost/dll/import.hpp>
#include <experimental/filesystem>

namespace lightdb::extensibility {

auto Load(const std::string &name, const std::experimental::filesystem::path &path="plugins") {
    auto plugin = boost::dll::import<functor::unaryfunctor>(
            std::string(path / name),
            name,
            boost::dll::load_mode::append_decorations);

    return plugin != nullptr
        ? std::shared_ptr<functor::naryfunctor<1>>(plugin.get(), [plugin](...) mutable { plugin.reset(); })
        : nullptr;
}

}; // namespace lightdb

#endif //LIGHTDB_EXTENSION_H
