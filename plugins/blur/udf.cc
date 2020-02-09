#include "udf.h"
#include "npp.h"
#include "ipp.h"

using namespace lightdb;

shared_reference<LightField> Blur::CPU::operator()(LightField& input) {
    const auto channels = 3u;
    IppiSize maskSize = {static_cast<int>(kernel_size_), static_cast<int>(kernel_size_)};

    auto &data = dynamic_cast<physical::CPUDecodedFrameData&>(input);
    physical::CPUDecodedFrameData output(data.configuration(), data.geometry());

    for(auto& frame: data.frames()) {
        Allocate(frame->height(), frame->width(), channels);
        IppiSize size{static_cast<int>(frame->width()), static_cast<int>(frame->height())};

        auto y_in = reinterpret_cast<const unsigned char*>(frame->data().data());
        auto uv_in = y_in + frame_size_;

        output.frames().emplace_back(LocalFrameReference::make<LocalFrame>(*frame, frame->data().size()));
        auto y_out = output.frames().back()->data().data();
        auto uv_out = y_out + frame_size_;

        // NV12 -> RGB
        CHECK_EQ(ippiYCbCr420ToRGB_8u_P2C3R(y_in, frame->width(), uv_in, frame->width(), rgb_.data(), channels * frame->width(), size), ippStsNoErr);

        // Blur
        CHECK_EQ(ippiFilterBoxBorder_8u_C3R(rgb_.data(), channels * frame->width(), rgb_blur_.data(), channels * frame->width(), size, maskSize, ippBorderRepl, nullptr, mask_.data()), ippStsNoErr);

        // RGB -> NV12
        CHECK_EQ(ippiRGBToYCbCr420_8u_C3P2R(rgb_blur_.data(), channels * frame->width(), (Ipp8u*)y_out, frame->width(), (Ipp8u*)uv_out, frame->width(), size), ippStsNoErr);
    }

    return output;
}

/*
 * //TODO Needs CUDA 10.0
shared_reference<LightField> Blur::GPU::operator()(LightField& input) {
    const auto channels = 3u;

    auto &data = dynamic_cast<physical::GPUDecodedFrameData&>(input);
    physical::GPUDecodedFrameData output;

    for(auto& frame: data.frames()) {
        auto frame_size_ = frame->height() * frame->width();


        auto cuda = frame->cuda();

        int rgb_pitch;
        auto rgb_frame = nppiMalloc_8u_C4(frame->width(), frame->height(), &rgb_pitch);

        int blur_pitch;
        auto blur_frame = nppiMalloc_8u_C4(frame->width(), frame->height(), &blur_pitch);

        //auto rgb_frame = CudaFrame(frame->height(), frame->width(), NV_ENC_PIC_STRUCT_FRAME); // Can just be a nppalloc
        //auto blur_frame = CudaFrame(frame->height(), frame->width(), NV_ENC_PIC_STRUCT_FRAME);
        auto output_frame = CudaFrame(frame->height(), frame->width(), NV_ENC_PIC_STRUCT_FRAME);

        NppiSize size{static_cast<int>(frame->width()), static_cast<int>(frame->height())};
        NppiSize blur_size{static_cast<int>(frame->width()), static_cast<int>(frame->height())};

        auto y_data = cuda->handle();
       // auto uv_data = y_data + frame_size_;
        auto uv_data = y_data + cuda->pitch() * cuda->height();
        const Npp8u *const source[2] = {(Npp8u*)y_data, (Npp8u*)uv_data};


        NppiSize mask_size = {5,5};
        NppiPoint anchor = {4, 4};

        // NV12 -> RGBA
        //assert(
        //nppiNV21ToBGR_8u_P2C4R(source, frame->width(), (Npp8u*)output_frame.handle(), channels * frame->width(), size);

            auto result = nppiNV21ToBGR_8u_P2C4R(source, cuda->pitch(), rgb_frame, rgb_pitch, size);
            printf("%d %d\n", result, result == NPP_STEP_ERROR);

            result = nppiFilterBox_8u_C4R(rgb_frame, rgb_pitch, blur_frame, blur_pitch, blur_size,
                    mask_size, anchor);
            printf("%d %d\n", result, result == NPP_STEP_ERROR);

        //result = nppiBGRToNV

        nppiFree(blur_frame);
        nppiFree(rgb_frame);

                //== ippStsNoErr);
        //ippiYCbCr420ToRGB_8u_P2C3R
        //nppiYCbCr420ToRG
        //nppiYCbCr411_8u_P2P3R()
          //      nppiYUV420ToRGB
    }

    return output;
}
*/

Blur blur;
