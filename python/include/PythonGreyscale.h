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
                  _kernelSize(size)
            { }
            void Allocate(const unsigned int height, const unsigned int width, const unsigned int channels) {
                if (_rgbSize != channels * height * width) {
                    _frameSize = height * width;
                    _rgbSize = channels * _frameSize;
                    printf("Resize rgb vector\n");
                    _rgb.resize(_rgbSize);
                    printf("Resized\n");
                    printf("Resize rgbGreyscale vector\n");
                    _rgbGreyscale.resize(_rgbSize);
                    printf("Resized Greyscale\n");
                    // printf("%d\n", _rgbSize);
                    // _mask.resize(_rgbSize);
                }
            }

            lightdb::shared_reference<lightdb::LightField> operator()(lightdb::LightField& field) override;

        private:
            unsigned int _kernelSize;
            unsigned int _rgbSize;
            unsigned int _frameSize;
            std::vector<unsigned char> _rgb;
            std::vector<unsigned char> _rgbGreyscale;
            std::vector<unsigned char> _mask;   
        };

    public:
        explicit Greyscale() : functor::unaryfunctor("Greyscale", GPU()) {}    
    };
}

#endif //LIGHTDB_GREYSCALE_H