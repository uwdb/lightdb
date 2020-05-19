#ifndef LIGHTDB_BLUR_LIBRARY_H
#define LIGHTDB_BLUR_LIBRARY_H

#include "Functor.h"

class Blur: public lightdb::functor::unaryfunctor {
    /*
    class GPU: public lightdb::functor::unaryfunction
    {
    public:
        GPU() : GPU(2) { }
        explicit GPU(const unsigned int size)
                : lightdb::functor::unaryfunction(lightdb::physical::DeviceType::GPU,
                                                  lightdb::Codec::hevc(),
                                                  true),
                  size_(size)
        { }

        lightdb::shared_reference<lightdb::LightField> operator()(lightdb::LightField& field) override;

    private:
        const unsigned int size_;
    };
*/
    class CPU: public lightdb::functor::unaryfunction
    {
    public:
        CPU() : CPU(3) { }
        explicit CPU(const unsigned int kernel_size)
                : lightdb::functor::unaryfunction(lightdb::physical::DeviceType::CPU,
                                                  lightdb::Codec::raw(),
                                                  true),
                  kernel_size_(kernel_size)
        { }

        void Allocate(const unsigned int height, const unsigned int width, const unsigned int channels) {
            if(rgb_size_ != channels * height * width) {
                frame_size_ = height * width;
                rgb_size_ = channels * frame_size_;
                rgb_.resize(rgb_size_);
                rgb_blur_.resize(rgb_size_);
                mask_.resize(rgb_size_);
                //scaled_.resize(total_size_);
                //planes_.resize(total_size_);
            }
        }


        unsigned int kernel_size(unsigned int kernel_size) { return kernel_size_ = kernel_size; }

        lightdb::shared_reference<lightdb::LightField> operator()(lightdb::LightField& field) override;

    private:
        unsigned int kernel_size_;
        unsigned int rgb_size_;
        unsigned int frame_size_;
        std::vector<unsigned char> rgb_;
        std::vector<unsigned char> rgb_blur_;
        std::vector<unsigned char> mask_;
    };

public:
    explicit Blur() : lightdb::functor::unaryfunctor("blur", CPU()) { }
};

#endif // LIGHTDB_BLUR_LIBRARY_H
