#ifndef VISUALCLOUD_ENCODING_H
#define VISUALCLOUD_ENCODING_H

#include <optional>
#include <fstream>

namespace visualcloud {

using bytestring = std::vector<char>;

class EncodedLightField {
public:
    explicit EncodedLightField(const std::string &filename)
        : EncodedLightField(static_cast<std::unique_ptr<std::istream>>(std::make_unique<std::ifstream>(filename.c_str())))
    { }

    explicit EncodedLightField(std::unique_ptr<std::istream> stream)
        : stream_(std::move(stream))
    { }

    std::istream& stream() const { return *stream_; }
    const bytestring& bytes() { return static_cast<const bytestring&>(*this); }

    operator const bytestring&() {
        return *(data_.has_value()
            ? data_
            : data_ = bytestring(std::istream_iterator<char>{*stream_},
                                      std::istream_iterator<char>{}));
    }

private:
    const std::unique_ptr<std::istream> stream_;
    std::optional<bytestring> data_;
};

};

#endif //VISUALCLOUD_ENCODING_H
