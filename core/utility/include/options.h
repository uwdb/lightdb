#ifndef LIGHTDB_OPTIONS_H
#define LIGHTDB_OPTIONS_H

#include <any>
#include <unordered_map>

namespace lightdb {
    template<typename TKey=std::string, typename TValue=std::any>
    class options: public std::unordered_map<TKey, TValue> {
    public:
        using std::unordered_map<TKey, TValue>::unordered_map;

        struct Encoding {
            static constexpr const char* Codec = "Codec";
            static constexpr const char* GOPSize = "GOP";
        };

        std::optional<TValue> get(const TKey& key) const {
            auto value = this->find(key);
            return value != this->end()
                ? std::optional<TValue>{(*value).second}
                : std::optional<TValue>{};
        }
    };

    template<typename TKey=std::string, typename TValue=std::any>
    class OptionContainer {
    public:
        virtual const lightdb::options<TKey, TValue> &options() const = 0;
        const std::optional<TValue> get_option(const TKey& key) const { return options().get(key); }
    };

    using EncodeOptions = options<>::Encoding;
} // namespace lightdb

#endif //LIGHTDB_OPTIONS_H
