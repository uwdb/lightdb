#include "AssertVideo.h"

CudaFrame CREATE_BLACK_FRAME(const Configuration &configuration)
{
    CUdeviceptr handle;
    size_t pitch;

    EXPECT_EQ(cuMemAllocPitch(
            &handle,
            &pitch,
            configuration.width,
            configuration.height * 3 / 2,
            8), CUDA_SUCCESS);

    // Set frame to black
    //   Chroma planes
    EXPECT_EQ(cuMemsetD2D8(handle, pitch, 128, configuration.width, configuration.height * 3 / 2), CUDA_SUCCESS);
    //   Luma plane
    EXPECT_EQ(cuMemsetD2D8(handle, pitch, 16, configuration.width, configuration.height), CUDA_SUCCESS);

    return CudaFrame(configuration.height, configuration.width,
                     NV_ENC_PIC_STRUCT_FRAME,
                     handle, static_cast<unsigned int>(pitch));
}

void ASSERT_BLACK_FRAME(const DecodedFrame &frame)
{
    auto cuda = CudaDecodedFrame(frame);
    auto local = LocalFrame(cuda);

    for(auto y = 0u; y < local.height(); y++)
        for(auto x = 0u; x < local.width(); x++)
            ASSERT_EQ(local(x, y), 16); // Black Y luma

    for(auto y = local.height(); y < local.height() * 3 / 2; y++)
        for(auto x = 0u; x < local.width(); x++)
            ASSERT_EQ(local(x, y), 128); // Black UV chroma
}

std::string TRANSCODE_RAW_TO_H264(const std::string& filename, const size_t height, const size_t width, const size_t framerate)
{
    auto output_filename = filename + ".h264";
    auto command = std::string("ffmpeg -y -hide_banner -loglevel error ") +
            " -f rawvideo -pix_fmt nv12 " +
            " -s:v " + std::to_string(width) + 'x' + std::to_string(height) +
            " -r " + std::to_string(framerate) +
            " -i " + filename +
            ' ' + output_filename;

    EXPECT_EQ(system(command.c_str()), 0);

    return output_filename;
}
