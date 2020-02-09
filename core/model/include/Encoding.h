#ifndef LIGHTDB_ENCODING_H
#define LIGHTDB_ENCODING_H

#include "Geometry.h"
#include <glog/logging.h>
#include <memory>
#include <optional>
#include <experimental/filesystem>
#include <fstream>

namespace lightdb {

    using bytestring = std::vector<char>;

    class EncodedLightFieldData {
    public:
        virtual ~EncodedLightFieldData() = default;

        //TODO combine these into a single struct
        virtual const std::vector<std::shared_ptr<bytestring>> segments() = 0;
        virtual const CompositeVolume volume() const = 0;

        virtual const std::shared_ptr<bytestring> bytes() = 0;
        virtual void write(const std::string &filename) = 0;
    };

    using EncodedLightField = std::shared_ptr<EncodedLightFieldData>;


    class SingletonFileEncodedLightField: public EncodedLightFieldData {
    public:
        static EncodedLightField create(const std::string &filename, const Volume &volume) {
            return std::shared_ptr<SingletonFileEncodedLightField>(new SingletonFileEncodedLightField{filename, volume});
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
            std::experimental::filesystem::copy_file(filename_, filename, std::experimental::filesystem::copy_options::overwrite_existing);
            //std::ofstream fout{filename, std::ofstream::out | std::ofstream::binary};
            //fout.write(bytes()->data(), bytes()->size());
        }

        const CompositeVolume volume() const override {
            return {volume_};
        }

    protected:
        explicit SingletonFileEncodedLightField(const std::string &filename, const Volume &volume)
            : filename_(filename), volume_(volume)
        { }

    private:
        std::string filename_;
        std::vector<std::shared_ptr<bytestring>> data_;
        const CompositeVolume volume_;
    };

    class CompositeMemoryEncodedLightField: public EncodedLightFieldData {
    public:
        //TODO should just take a vector of bytestrings
        static EncodedLightField create(std::vector<std::shared_ptr<bytestring>> &encodings, const CompositeVolume &volume) {
            return std::shared_ptr<CompositeMemoryEncodedLightField>(new CompositeMemoryEncodedLightField(encodings, volume));
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

        const CompositeVolume volume() const override {
            return volume_;
        }

        void write(const std::string &filename) override {
            size_t position = filename.find('*');

            //TODO serializing a singleton should kind of still create a directory and persist metadata
            if(position == std::string::npos) {
                std::ofstream fout{filename, std::ofstream::out | std::ofstream::binary};
                for (auto bytes: data_)
                    fout.write(bytes->data(), bytes->size());
            } else {
                for(auto i = 0u; i < segments().size(); i++) {
                    auto current_filename = std::string{filename}.replace(position, 1, std::to_string(i));
                    std::ofstream fout{current_filename, std::ofstream::out | std::ofstream::binary};
                    fout.write(segments()[i]->data(), segments()[i]->size());
                }
            }
        }

    protected:
        explicit CompositeMemoryEncodedLightField(const std::vector<std::shared_ptr<bytestring>> &data,
                                                  const CompositeVolume &volume)
                : data_(data), volume_(volume)
        { }

    private:
        std::vector<std::shared_ptr<bytestring>> data_;
        const CompositeVolume volume_;
    };

    class SingletonMemoryEncodedLightField: public EncodedLightFieldData {
    public:
        //TODO should just take a vector of bytestrings
        static EncodedLightField create(std::shared_ptr<bytestring> &encoding, const CompositeVolume &volume) {
            return std::shared_ptr<SingletonMemoryEncodedLightField>(new SingletonMemoryEncodedLightField(encoding, volume));
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

        const CompositeVolume volume() const override {
            return {volume_};
        }

        void write(const std::string &filename) override {
            //TODO serializing a singleton should kind of still create a directory and persist metadata
            std::ofstream fout{filename, std::ofstream::out | std::ofstream::binary};
            fout.write(bytes()->data(), bytes()->size());
        }

    protected:
        explicit SingletonMemoryEncodedLightField(const std::shared_ptr<bytestring> &data, const CompositeVolume &volume)
            : data_({data}), volume_(volume)
        { }

    private:
        std::vector<std::shared_ptr<bytestring>> data_;
        const CompositeVolume volume_;
    };
};

#endif //LIGHTDB_ENCODING_H
