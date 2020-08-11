#include "PythonUnary.h"
#include "Format.h"
#include "npp.h"
#include <boost/python/numpy.hpp>
#include <iostream>
#include <vector>

namespace np = boost::python::numpy;

namespace lightdb::python {
    shared_reference <LightField> PythonUnary::CPU::operator()(LightField &input) {
        np::initialize();
        const auto channels = 3u;
        auto &data = dynamic_cast<lightdb::physical::CPUDecodedFrameData&>(input);
        lightdb::physical::CPUDecodedFrameData output(data.configuration(), data.geometry());

         for(auto& frame: data.frames()) {

            //ToDo: add equality operator
            // frame->format() == lightdb::video::Format::nv12()
            Allocate(frame->height(), frame->width(), channels);
            IppiSize size{static_cast<int>(frame->width()), static_cast<int>(frame->height())};
            auto y_in = reinterpret_cast<const unsigned char*>(frame->data().data());
            auto uv_in = y_in + frameSize_;
            output.frames().emplace_back(LocalFrameReference::make<LocalFrame>(*frame, frame->data().size(), frame->format()));
            auto y_out = output.frames().back()->data().data();
            auto uv_out = y_out + frameSize_;

            // NV12 -> RGB
            nv12ToRgb(frame, y_in, uv_in, channels, size);
            // RGB --> Numpy 
            np::dtype dtype = np::dtype::get_builtin<unsigned char>();
            np::ndarray py_array = np::from_data(
                                        rgb_.data(), 
                                        dtype, 
                                        boost::python::make_tuple(channels, frame->width(), frame->height()),
                                        boost::python::make_tuple(1 , sizeof(unsigned char) * channels, sizeof(unsigned char) * channels * frame->width()),
                                        boost::python::object());

            np::ndarray swap = py_array.transpose();
            boost::python::call<void>(callable_, swap); 

            // RGB -> NV12
            CHECK_EQ(ippiRGBToYCbCr420_8u_C3P2R(rgb_.data(), channels * frame->width(), const_cast<Ipp8u*>(reinterpret_cast<const Ipp8u*>(y_out)), frame->width(), 
                    const_cast<Ipp8u*>(reinterpret_cast<const Ipp8u*>(uv_out)), frame->width(), size), ippStsNoErr);                     
        }
        return output;
    }

    void PythonUnary::CPU::nv12ToRgb(auto& frame, auto y_in, auto uv_in, const auto channels, const IppiSize& size) {
        CHECK_EQ(ippiYCbCr420ToRGB_8u_P2C3R(const_cast<Ipp8u*>(reinterpret_cast<const Ipp8u*>(y_in)), frame->width(), const_cast<Ipp8u*>(reinterpret_cast<const Ipp8u*>(uv_in)), 
                frame->width(), rgb_.data(), channels * frame->width(), size), ippStsNoErr);
    }
}
