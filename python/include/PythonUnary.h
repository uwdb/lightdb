#ifndef LIGHTDB_PYTHON_UNARY_H
#define LIGHTDB_PYTHON_UNARY_H


#include "ipp.h"
#include "Functor.h"
#include <Python.h>


namespace lightdb::python {
    class PythonUnary :  public lightdb::functor::unaryfunctor {
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
                // boost::python::incref(callable.ptr()); 
            }

            void Allocate(const unsigned int height, const unsigned int width, const unsigned int channels) {
                if (rgbSize_ != channels * height * width) {
                    frameSize_ = height * width;
                    rgbSize_ = channels * frameSize_;
                    rgb_.resize(rgbSize_);
                }
            }

            void nv12ToRgb(auto& frame, auto y_in, auto uv_in, const auto channels, const IppiSize& size); 

            lightdb::shared_reference<lightdb::LightField> operator()(lightdb::LightField& field) override;

        private:
            PyObject* const callable_;
            unsigned int rgbSize_;
            unsigned int frameSize_;
            std::vector<unsigned char> rgb_;  
        };

    public:
        explicit PythonUnary(PyObject* const callable, const bool deterministic=true) : functor::unaryfunctor("Unary", CPU(callable, deterministic)) {}      
    };

}

#endif //LIGHTDB__PYTHON_UNARY_H