#include "udf.h"
#include "ipp.h"

using namespace lightdb;

YOLO::CPU::CPU()
        : CPU("/home/bhaynes/projects/darknet/cfg/tiny-yolo.cfg",
              "/home/bhaynes/projects/darknet/tiny-yolo.weights",
              "/home/bhaynes/projects/darknet/cfg/coco.data")
{ }

//int pow2_ceiling(unsigned int value) { return 2 << (unsigned int)log2l(value); }

shared_reference<LightField> YOLO::CPU::operator()(LightField& input) {
    const auto channels = 3u;
    std::vector<char> output;
    auto &data = dynamic_cast<physical::CPUDecodedFrameData&>(input);

    for(auto& frame: data.frames()) {
        Allocate(frame->height(), frame->width(), channels);

        auto y_data = reinterpret_cast<const unsigned char*>(frame->data().data());
        auto uv_data = y_data + frame_size_;
        IppiSize size{static_cast<int>(frame->width()), static_cast<int>(frame->height())};
        //IppiSize size{pow2_ceiling(frame->width()), pow2_ceiling(frame->height())};

        // NV12 -> RGB
        //assert(
                ippiYCbCr420ToRGB_8u_P2C3R(y_data, frame->width(), uv_data, frame->width(), rgb_.data(), channels * frame->width(), size)
                ;//== ippStsNoErr);

        // RGBRGBRGB -> RRRBBBGGG
        assert(ippiCopy_8u_C3P3R(rgb_.data(), 3 * frame->width(), std::initializer_list<unsigned char*>(
                {planes_.data(), planes_.data() + frame_size_, planes_.data() + 2*frame_size_}).begin(),
                frame->width(), size) == ippStsNoErr);

        // uchar -> float
        assert(ippsConvert_8u32f(planes_.data(), scaled_.data(), total_size_) == ippStsNoErr);

        // float -> scaled float (x / 255)
        assert(ippsDivC_32f_I(255.f, scaled_.data(), total_size_) == ippStsNoErr);

        network_detect(network_,
                       {size.width, size.height, channels, scaled_.data()},
                       threshold_,
                       hierarchical_threshold_,
                       nms_,
                       boxes_,
                       probabilities_);

        if(true)
        for(auto i = 0u; i < box_count_; i++)
            for(auto j = 0u; j < metadata_.classes; j++)
                if(probabilities_[i][j] > 0.001)
                    {
                    printf("%s,%.0f,%.0f,%.0f,%.0f,%.3f\n", metadata_.names[j], boxes_[i].x, boxes_[i].y, boxes_[i].w, boxes_[i].h, probabilities_[i][j]);
                    output.insert(output.end(), reinterpret_cast<char*>(&boxes_[i]),
                                                reinterpret_cast<char*>(&boxes_[i + 1]));
                    }
        }

    return physical::CPUEncodedFrameData(Codec::boxes(), output);
}

YOLO yolo;
