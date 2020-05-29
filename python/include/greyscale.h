#ifndef LIGHTDB_GREYSCALE_H
#define LIGHTDB_GREYSCALE_H

#include "Functor.h"


namespace lightdb {
    class Greyscale :  public functor::unaryfunctor {
        class GPU : public functor::unaryfunction {
        public:
            GPU() : GPU(2) { }
            explicit GPU(const unsigned int size)
                : lightdb::functor::unaryfunction(lightdb::physical::DeviceType::GPU,
                                                  lightdb::Codec::hevc(),
                                                  true),
                  kernel_size_(size)
            { }
            void Allocate(const unsigned int height, const unsigned int width, const unsigned int channels) {
                if (rgb_size_ != channels * height * width) {
                    frame_size_ = height * width;
                    rgb_size_ = channels * frame_size_;
                    rgb_.resize(rgb_size_);
                    rgb_greyscale_.resize(rgb_size_);
                    mask_.resize(rgb_size_);
                }
            }

            lightdb::shared_reference<lightdb::LightField> operator()(lightdb::LightField& field) override;

        private:
            unsigned int kernel_size_;
            unsigned int rgb_size_;
            unsigned int frame_size_;
            std::vector<unsigned char> rgb_;
            std::vector<unsigned char> rgb_greyscale_;
            std::vector<unsigned char> mask_;   
        };

    public:
        explicit Greyscale() : functor::unaryfunctor("Greyscale", GPU()) {}    
    };
}

#endif //LIGHTDB_GREYSCALE_H