#ifndef LIGHTDB_PYTHON_GREYSCALE_H
#define LIGHTDB_PYTHON_GREYSCALE_H

#include "Functor.h"


namespace lightdb::python {
    class PythonGreyscale :  public lightdb::functor::unaryfunctor {
        class CPU : public lightdb::functor::unaryfunction {
        public:
            explicit CPU(PyObject* const callable, const bool deterministic=true)
                : lightdb::functor::unaryfunction(lightdb::physical::DeviceType::CPU,
                                                  lightdb::Codec::raw(),
                                                  deterministic),
                  callable_(callable)
            { 
                CHECK_NOTNULL(callable);
                callable->ob_refcnt++; 
            }

            void Allocate(const unsigned int height, const unsigned int width, const unsigned int channels) {
                if (rgbSize_ != channels * height * width) {
                    frameSize_ = height * width;
                    rgbSize_ = channels * frameSize_;
                    rgb_.resize(rgbSize_);
                    mask_.resize(rgbSize_);
                }
            }

            lightdb::shared_reference<lightdb::LightField> operator()(lightdb::LightField& field) override;

        private:
            PyObject* const callable_;
            unsigned int kernelSize_;
            unsigned int rgbSize_;
            unsigned int frameSize_;
            std::vector<unsigned char> rgb_;
            std::vector<unsigned char> mask_;   
        };

    public:
        explicit PythonGreyscale(PyObject* const callable, const bool deterministic=true) : functor::unaryfunctor("Greyscale", CPU(callable, deterministic)) {}      
    };

}

#endif //LIGHTDB__PYTHON_GREYSCALE_H