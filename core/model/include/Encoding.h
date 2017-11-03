#ifndef VISUALCLOUD_ENCODING_H
#define VISUALCLOUD_ENCODING_H

#include <glog/logging.h>
#include <memory>
#include <optional>
#include <fstream>

namespace visualcloud {

    using bytestring = std::vector<char>;

    class EncodedLightFieldData {
    public:
        virtual ~EncodedLightFieldData() { }

        virtual const std::vector<std::shared_ptr<bytestring>> segments() = 0;
        virtual const std::shared_ptr<bytestring> bytes() = 0;
        virtual void write(const std::string &filename) = 0;
    };

    using EncodedLightField = std::shared_ptr<EncodedLightFieldData>;


    class SingletonFileEncodedLightField: public EncodedLightFieldData {
    public:
        static EncodedLightField create(const std::string &filename) {
            return std::shared_ptr<SingletonFileEncodedLightField>(new SingletonFileEncodedLightField{filename});
        }

        virtual ~SingletonFileEncodedLightField() { }

        const std::string &filename() const { return filename_; }

        const std::vector<std::shared_ptr<bytestring>> segments() override {
            return data_;
        }

        const std::shared_ptr<bytestring> bytes() override {
            if(data_.size() == 0) {
                std::ifstream stream{filename_};
                auto b = std::make_shared<bytestring>(std::istreambuf_iterator<char>{ stream },
                             std::istreambuf_iterator<char>{});
                LOG(INFO) << "Encoding materializing; read " << b->size() << " bytes";
                data_.push_back(b);
            }

            return data_[0];
        }

        void write(const std::string &filename) override {
            //TODO serializing a singleton should kind of still create a directory and persist metadata
            std::ofstream fout{filename, std::ofstream::out | std::ofstream::binary};
            fout.write(bytes()->data(), bytes()->size());
        }

    protected:
        explicit SingletonFileEncodedLightField(const std::string &filename)
            : filename_(filename)
        { }

    private:
        std::string filename_;
        std::vector<std::shared_ptr<bytestring>> data_;
    };

    class CompositeMemoryEncodedLightField: public EncodedLightFieldData {
    public:
        //TODO should just take a vector of bytestrings
        static EncodedLightField create(std::vector<std::shared_ptr<bytestring>> &encodings) {
            return std::shared_ptr<CompositeMemoryEncodedLightField>(new CompositeMemoryEncodedLightField(encodings));
        }

        ~CompositeMemoryEncodedLightField() { }

        const std::vector<std::shared_ptr<bytestring>> segments() { return data_; }
        const std::shared_ptr<bytestring> bytes() {
            //TODO improve efficiency
            if(segments().size() == 1)
                return segments()[0];
            else {
                std::shared_ptr<bytestring> result{};
                for(auto &v: segments())
                    result->insert(result->end(), v->begin(), v->end());
                return result;
            }
        }

        void write(const std::string &filename) override {
            size_t position = filename.find('*');

            //TODO serializing a singleton should kind of still create a directory and persist metadata
            if(position == std::string::npos) {
                std::ofstream fout{filename, std::ofstream::out | std::ofstream::binary};
                for (auto bytes: data_)
                    fout.write(bytes->data(), bytes->size());
            } else {
                for(auto i = 0; i < segments().size(); i++) {
                    auto current_filename = std::string{filename}.replace(position, 1, std::to_string(i));
                    std::ofstream fout{current_filename, std::ofstream::out | std::ofstream::binary};
                    fout.write(segments()[i]->data(), segments()[i]->size());
                }
            }
        }

    protected:
        explicit CompositeMemoryEncodedLightField(const std::vector<std::shared_ptr<bytestring>> &data)
                : data_(data)
        { }

    private:
        std::vector<std::shared_ptr<bytestring>> data_;
    };

    class SingletonMemoryEncodedLightField: public EncodedLightFieldData {
    public:
        //TODO should just take a vector of bytestrings
        static EncodedLightField create(std::shared_ptr<bytestring> &encoding) {
            return std::shared_ptr<SingletonMemoryEncodedLightField>(new SingletonMemoryEncodedLightField(encoding));
        }

        ~SingletonMemoryEncodedLightField() { }

        const std::vector<std::shared_ptr<bytestring>> segments() { return data_; }
        const std::shared_ptr<bytestring> bytes() {
            //TODO improve efficiency
            if(segments().size() == 1)
                return segments()[0];
            else {
                std::shared_ptr<bytestring> result{};
                for(auto &v: segments())
                    result->insert(result->end(), v->begin(), v->end());
                return result;
            }
        }

        void write(const std::string &filename) override {
            //TODO serializing a singleton should kind of still create a directory and persist metadata
            std::ofstream fout{filename, std::ofstream::out | std::ofstream::binary};
            fout.write(bytes()->data(), bytes()->size());
        }

    protected:
        explicit SingletonMemoryEncodedLightField(const std::shared_ptr<bytestring> &data)
            : data_({data})
        { }

    private:
        std::vector<std::shared_ptr<bytestring>> data_;
    };
};

#endif //VISUALCLOUD_ENCODING_H
