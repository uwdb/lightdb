#ifndef LIGHTDB_EXTENSION_H
#define LIGHTDB_EXTENSION_H

#include "Functor.h"
#include <boost/dll/import.hpp>

namespace lightdb::extensibility {

auto Load(const std::string &name, const std::string &filename="plugins") {
    auto plugin = boost::dll::import<functor::unaryfunctor>(
            filename + "/" + name,
            name,
            boost::dll::load_mode::append_decorations);

    return plugin != nullptr
        ? std::shared_ptr<functor::naryfunctor<1>>(plugin.get(), [plugin](...) mutable { plugin.reset(); })
        : nullptr;
}

}; // namespace lightdb

#endif //LIGHTDB_EXTENSION_H
