#ifndef VISUALCLOUD_ENCODING_H
#define VISUALCLOUD_ENCODING_H

#include <optional>
#include <fstream>

namespace visualcloud {

class EncodedLightFieldData;
class SingletonEncodedLightField;

using bytestring = std::vector<char>;
using EncodedLightField = std::shared_ptr<EncodedLightFieldData>;

class EncodedLightFieldData {
public:
    static EncodedLightField create(std::vector<std::unique_ptr<std::istream>> &&streams) {
        return std::shared_ptr<EncodedLightFieldData>(new EncodedLightFieldData(std::move(streams)));
    }

    static EncodedLightField create(std::vector<bytestring> &&encodings) {
        return std::shared_ptr<EncodedLightFieldData>(new EncodedLightFieldData(std::move(encodings)));
    }

    virtual ~EncodedLightFieldData() { }

    EncodedLightFieldData(const EncodedLightFieldData&) = delete;
    EncodedLightFieldData(const EncodedLightFieldData&&) = delete;

    const std::vector<std::unique_ptr<std::istream>>& streams() const { return streams_; }
    const std::vector<bytestring> encodings() {
        if(!data_.has_value()) {
            data_ = std::vector<bytestring>{};
            for(auto &stream: streams_) {
                // TODO(bhaynes)
                bytestring b(std::istreambuf_iterator<char> { *stream },
                             std::istreambuf_iterator<char>{});
                LOG(INFO) << "Encoding materializing; read " << b.size() << " bytes";
                data_.value().emplace_back(b);
            }
        }

        return data_.value();
    }
    virtual const bytestring bytes() {
        //TODO improve efficiency
        if(encodings().size() == 1)
            return encodings().at(0);
        else {
            bytestring result;
            for(auto &v: encodings())
                result.insert(result.end(), v.begin(), v.end());
            return result;
        }
    }

    virtual void write(const std::string &filename) {
        throw std::runtime_error("Serializing a composite not currently supported."); //TODO support this...
    }

    //TODO this whole singleton encoding thing is a hack
    virtual const SingletonEncodedLightField* singleton() const {
        return {};
    }

protected:
    EncodedLightFieldData(std::vector<std::unique_ptr<std::istream>> &&streams)
        : streams_(std::move(streams))
    { }

    EncodedLightFieldData(std::vector<bytestring> &&encodings)
            : streams_{}, data_(std::move(encodings))
    { }

private:
    const std::vector<std::unique_ptr<std::istream>> streams_;
    std::optional<std::vector<bytestring>> data_;
};

using CompositeEncodedLightField = EncodedLightFieldData;

class SingletonEncodedLightField: public EncodedLightFieldData {
public:
    static EncodedLightField create(const std::string &filename) {
        return std::shared_ptr<SingletonEncodedLightField>(new SingletonEncodedLightField{filename});
    }

    static EncodedLightField create(std::unique_ptr<std::istream> &&stream) {
        return std::shared_ptr<SingletonEncodedLightField>(new SingletonEncodedLightField(std::move(stream)));
    }

    ~SingletonEncodedLightField() { }

    std::istream& stream() const { return *streams().at(0); }

    const SingletonEncodedLightField* singleton() const override {
        return this;
    }

    void write(const std::string &filename) override {
        //TODO serializing a singleton should kind of still create a directory and persist metadata
        std::ofstream fout{filename, std::ofstream::out | std::ofstream::binary};
        fout.write(bytes().data(), bytes().size());
    }

protected:
    explicit SingletonEncodedLightField(const std::string &filename)
        : SingletonEncodedLightField(static_cast<std::unique_ptr<std::istream>>(
                                             std::make_unique<std::ifstream>(filename.c_str(),
                                                                             std::ios::in | std::ios::binary)))
    { }

    explicit SingletonEncodedLightField(std::unique_ptr<std::istream> &&stream)
        : EncodedLightFieldData(singleton(std::move(stream)))
    { }

private:
    static std::vector<std::unique_ptr<std::istream>> singleton(std::unique_ptr<std::istream> &&stream) {
        std::vector<std::unique_ptr<std::istream>> vector;
        vector.emplace_back(std::move(stream));
        return std::move(vector);
    }
};

};

#endif //VISUALCLOUD_ENCODING_H
