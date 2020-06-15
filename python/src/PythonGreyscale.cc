#include <boost/python/numpy.hpp>
#include <iostream>
#include "PythonGreyscale.h"
#include "npp.h"
#include "ipp.h"
#include "Format.h"


namespace lightdb {
    shared_reference <LightField> Greyscale::GPU::operator()(LightField &input) {

        const auto channels = 2u;
        IppiSize makeSize = {static_cast<int>(_kernelSize), static_cast<int>(_kernelSize)};
        auto &data = dynamic_cast<physical::GPUDecodedFrameData&>(input);
        physical::CPUDecodedFrameData output(data.configuration(), data.geometry());

         for(auto& frame: data.frames()) {
            //TODO add NV12 helper utilities
            // auto uv_offset = frame->width() * frame->height();
            // auto uv_height = frame->height() / 2;
            // auto cuda = frame->cuda();

            // CHECK_EQ(cuMemsetD2D8(cuda->handle() + uv_offset,
            //                         cuda->pitch(),
            //                         128,
            //                         cuda->width(),
            //                         uv_height), CUDA_SUCCESS);

            printf("Allocating memory\n");
            Allocate(frame->height(), frame->width(), channels);
            printf("Allocated memory\n");
            IppiSize size{static_cast<int>(frame->width()), static_cast<int>(frame->height())};
            LocalFrame cpuFrame(*(frame->cuda()));
            auto y_in = reinterpret_cast<const unsigned char*>(cpuFrame.data().data());
            auto uv_in = y_in + _frameSize;
            output.frames().emplace_back(LocalFrameReference::make<LocalFrame>(cpuFrame, cpuFrame.data().size(), video::Format::nv12()));
            auto y_out = output.frames().back()->data().data();
            auto uv_out = y_out + _frameSize;

            // NV12 -> RGB
            CHECK_EQ(ippiYCbCr420ToRGB_8u_P2C3R(y_in, frame->width(), uv_in, frame->width(), _rgb.data(), channels * frame->width(), size), ippStsNoErr);

            // RGB -> NV12
            CHECK_EQ(ippiRGBToYCbCr420_8u_C3P2R(_rgb.data(), channels * frame->width(), (Ipp8u*)y_out, frame->width(), (Ipp8u*)uv_out, frame->width(), size), ippStsNoErr);                        
        }
        return output;
        // return data;
    }
}