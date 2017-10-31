#ifndef VISUALCLOUD_ENCODING_H
#define VISUALCLOUD_ENCODING_H

#include <glog/logging.h>
#include <memory>
#include <optional>
#include <fstream>

namespace visualcloud {

class EncodedLightFieldData;
class SingletonEncodedLightField;

using bytestring = std::vector<char>;
using EncodedLightField = std::shared_ptr<EncodedLightFieldData>;

class EncodedLightFieldData {
public:
    virtual ~EncodedLightFieldData() { }

    virtual const std::vector<std::shared_ptr<bytestring>> encodings() = 0;
    virtual const bytestring bytes() = 0;
    virtual size_t size() = 0;
    virtual void write(const std::string &filename) = 0;
};

//using CompositeEncodedLightField = EncodedLightFieldData;
/*
class EncodedLightFieldDataXXX {
public:
    static EncodedLightField create(std::vector<std::shared_ptr<std::istream>> &&streams) {
        return std::shared_ptr<EncodedLightFieldData>(new EncodedLightFieldData(std::move(streams)));
    }

    static EncodedLightField create(std::vector<bytestring> &&encodings) {
        return std::shared_ptr<EncodedLightFieldData>(new EncodedLightFieldData(std::move(encodings)));
    }

    virtual ~EncodedLightFieldData() { }

    EncodedLightFieldData(const EncodedLightFieldData&) = delete;
    EncodedLightFieldData(const EncodedLightFieldData&&) = delete;

    const std::vector<std::shared_ptr<std::istream>>& streams() const { return streams_; }
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
    const bytestring bytes() {
        //TODO improve efficiency
        LOG(ERROR) << "foo";
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
    EncodedLightFieldData(std::vector<std::shared_ptr<std::istream>> &&streams)
        : streams_(std::move(streams))
    { }

    EncodedLightFieldData(std::vector<bytestring> &&encodings)
            : streams_{}, data_(std::move(encodings))
    { }

private:
    const std::vector<std::shared_ptr<std::istream>> streams_;
    std::optional<std::vector<bytestring>> data_;
};
 */

class SingletonFileEncodedLightField: public EncodedLightFieldData {
public:
    static EncodedLightField create(const std::string &filename) {
        return std::shared_ptr<SingletonFileEncodedLightField>(new SingletonFileEncodedLightField{filename});
    }

    //static EncodedLightField create(std::shared_ptr<std::istream> &&stream) {
    //    return std::shared_ptr<SingletonEncodedLightField>(new SingletonEncodedLightField(std::move(stream)));
    //}

    virtual ~SingletonFileEncodedLightField() { }

    //std::istream& stream() const { return *streams().at(0); }

    //const SingletonEncodedLightField* singleton() const override {
    //    return this;
    //}
    const std::string &filename() const { return filename_; }

    const std::vector<std::shared_ptr<bytestring>> encodings() override {
        return data_; //std::vector<bytestring>{std::vector{bytes()}};
    }

    size_t size() override {
        if(data_.size() == 0)
            bytes();
        return data_[0]->size();
    }

    const bytestring bytes() override {
        //if(!data_.has_value()) {
        if(data_.size() == 0) {
            std::ifstream stream{filename_};
            auto b = std::make_shared<bytestring>(std::istreambuf_iterator<char>{ stream },
                         std::istreambuf_iterator<char>{});
            LOG(INFO) << "Encoding materializing; read " << b->size() << " bytes";
            data_.push_back(b);
        }

        //return data_.value();
        return *data_[0];
    }

    void write(const std::string &filename) override {
        //TODO serializing a singleton should kind of still create a directory and persist metadata
        std::ofstream fout{filename, std::ofstream::out | std::ofstream::binary};
        auto materialized = bytes();
        fout.write(materialized.data(), materialized.size());
    }

protected:
    explicit SingletonFileEncodedLightField(const std::string &filename)
        //: EncodedLightFieldData(std::move(singleton(filename)))
        : filename_(filename)
    { }

private:
    std::string filename_;
    std::vector<std::shared_ptr<bytestring>> data_;
};

/*
class SingletonEncodedLightField: public EncodedLightFieldData {
public:
    static EncodedLightField create(const std::string &filename) {
        return std::shared_ptr<SingletonEncodedLightField>(new SingletonEncodedLightField{filename});
    }

    static EncodedLightField create(std::shared_ptr<std::istream> &&stream) {
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
        : EncodedLightFieldData(std::move(singleton(filename)))
    { }

    explicit SingletonEncodedLightField(std::shared_ptr<std::istream> &&stream)
        : EncodedLightFieldData(singleton(std::move(stream)))
    { }

private:
    //TODO collapse both of these into one function
    static std::vector<std::shared_ptr<std::istream>> singleton(std::shared_ptr<std::istream> &&stream) {
        std::vector<std::shared_ptr<std::istream>> vector;
        vector.emplace_back(std::move(stream));
        return std::move(vector);
    }

    static std::vector<std::shared_ptr<std::istream>> singleton(const std::string &filename) {
        //std::vector<std::shared_ptr<std::istream>> vector;
        //vector.emplace_back(std::make_shared<std::ifstream>(filename, std::ios::in | std::ios::binary));
        return {std::make_shared<std::ifstream>(filename, std::ios::in | std::ios::binary)};
    }
};
 */

    class CompositeMemoryEncodedLightField: public EncodedLightFieldData {
    public:
        //TODO should just take a vector of bytestrings
        static EncodedLightField create(std::vector<std::shared_ptr<bytestring>> &encodings) {
            return std::shared_ptr<CompositeMemoryEncodedLightField>(new CompositeMemoryEncodedLightField(encodings));
        }
        //static EncodedLightField create(std::vector<std::shared_ptr<bytestring>> &&encodings) {
        //    return std::shared_ptr<CompositeMemoryEncodedLightField>(new CompositeMemoryEncodedLightField(std::move(encodings)));
        //}

        ~CompositeMemoryEncodedLightField() { }

        const std::vector<std::shared_ptr<bytestring>> encodings() { return data_; }
        const bytestring bytes() {
            //TODO improve efficiency
            LOG(ERROR) << "foo";
            if(encodings().size() == 1)
                return *encodings()[0];
            else {
                bytestring result;
                for(auto &v: encodings())
                    result.insert(result.end(), v->begin(), v->end());
                return result; // std::make_shared<bytestring>(result);
            }
        }

        size_t size() override {
            //TODO way inefficient!
            return bytes().size();
        }

        void write(const std::string &filename) override {
            //TODO serializing a singleton should kind of still create a directory and persist metadata
            std::ofstream fout{filename, std::ofstream::out | std::ofstream::binary};
            auto materialized = bytes();
            fout.write(materialized.data(), materialized.size());
        }

    protected:
        explicit CompositeMemoryEncodedLightField(const std::vector<std::shared_ptr<bytestring>> &data)
                : data_(data)
        { }

    private:
        std::vector<std::shared_ptr<bytestring>> data_;
    };

};

#endif //VISUALCLOUD_ENCODING_H
