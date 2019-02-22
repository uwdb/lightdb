#include "extension.h"
#include "errors.h"
#include <boost/dll/import.hpp>

namespace lightdb::extensibility {

    std::shared_ptr<functor::unaryfunctor> Load(const std::string &name,
                                                const std::experimental::filesystem::path &path) {
        try {
            auto plugin = boost::dll::import<functor::unaryfunctor>(
                    std::string(path / name),
                    name,
                    boost::dll::load_mode::append_decorations);

            return plugin != nullptr
                   ? std::shared_ptr<functor::naryfunctor<1>>(plugin.get(), [plugin](...) mutable { plugin.reset(); })
                   : nullptr;
        } catch(boost::system::system_error &e) {
            throw PluginError(e.what(), name);
        }
    }

}; // namespace lightdb
