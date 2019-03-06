#ifndef LIGHTDB_ASSERTVIDEO_H
#define LIGHTDB_ASSERTVIDEO_H

#include "Frame.h"
#include "gtest/gtest.h"

#define DEFAULT_PSNR 30

////////////////////////////////////////////////////////////////////////////////////////////////////

using std::to_string;

#define _FRAME_COMMAND(filename, frame_count) \
     (std::string("resources/assert-frames.sh ") + \
                     (filename) + ' ' + \
                     to_string(frame_count))

#define ASSERT_VIDEO_FRAMES(filename, frame_count) { \
    auto command = _FRAME_COMMAND(filename, frame_count); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": expected " << (frame_count) << " frames"; }

#define EXPECT_VIDEO_FRAMES(filename, frame_count) { \
    auto command = _FRAME_COMMAND(filename, frame_count); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": expected " << (frame_count) << " frames"; }

////////////////////////////////////////////////////////////////////////////////////////////////////

#define _GOP_COMMAND(filename, gop_size) \
     (std::string("resources/assert-gop.sh ") + \
                     (filename) + ' ' + \
                     to_string(gop_size))

#define ASSERT_VIDEO_GOP(filename, gop_size) { \
    auto command = _GOP_COMMAND(filename, gop_size); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": expected GOP size " << (gop_size); }

#define EXPECT_VIDEO_GOP(filename, gop_size) { \
    auto command = _GOP_COMMAND(filename, gop_size); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": expected GOP size " << (gop_size); }

////////////////////////////////////////////////////////////////////////////////////////////////////

#define _VALID_VIDEO_COMMAND(filename) \
     (std::string("ffprobe -hide_banner -loglevel quiet ") + (filename))

#define ASSERT_VIDEO_VALID(filename) { \
    auto command = _VALID_VIDEO_COMMAND(filename); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": video could not be parsed"; }

#define EXPECT_VIDEO_VALID(filename) { \
    auto command = _VALID_VIDEO_COMMAND(filename); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": video could not be parsed"; }

////////////////////////////////////////////////////////////////////////////////////////////////////

#define _RESOLUTION_COMMAND(filename, height, width) \
     (std::string("resources/assert-resolution.sh ") + \
                     (filename) + ' ' + \
                     to_string(height) + ' ' + \
                     to_string(width))

#define ASSERT_VIDEO_RESOLUTION(filename, height, width) { \
    auto command = _RESOLUTION_COMMAND(filename, height, width); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected video resolution (expected " << (width) << 'x' << (height) << ')'; }

#define EXPECT_VIDEO_RESOLUTION(filename, height, width) { \
    auto command = _RESOLUTION_COMMAND(filename, height, width); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected video resolution (expected " << (width) << 'x' << (height) << ')'; }

////////////////////////////////////////////////////////////////////////////////////////////////////

#define _QUALITY_COMMAND(filename, reference_filename, minimum_psnr, \
                         reference_left, reference_top, reference_right, reference_bottom, \
                         source_left, source_top, source_right, source_bottom) \
     (std::string("resources/assert-quality.sh ") + \
                     (filename) + ' ' + \
                     (reference_filename) + ' ' + \
                     std::to_string(minimum_psnr) + ' ' + \
                     std::to_string(reference_left) + ' ' + std::to_string(reference_top) + ' ' + \
                     std::to_string(reference_right) + ' ' + std::to_string(reference_bottom) + ' ' + \
                     std::to_string(source_left) + ' ' + std::to_string(source_top) + ' ' + \
                     std::to_string(source_right) + ' ' + std::to_string(source_bottom))

#define _QUALITY_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,NAME,...) NAME
#define ASSERT_VIDEO_QUALITY(...) \
    _QUALITY_MACRO(__VA_ARGS__, _ASSERT_VIDEO_QUALITY11, 0, 0, 0, \
                                _ASSERT_VIDEO_QUALITY7, 0, 0, 0, \
                                _ASSERT_VIDEO_QUALITY3, 0, 0)(__VA_ARGS__)
#define EXPECT_VIDEO_QUALITY(...) \
    _QUALITY_MACRO(__VA_ARGS__, _EXPECT_VIDEO_QUALITY11, 0, 0, 0, \
                                _EXPECT_VIDEO_QUALITY7, 0, 0, 0,  \
                                _EXPECT_VIDEO_QUALITY3, 0, 0)(__VA_ARGS__)

#define _ASSERT_VIDEO_QUALITY3(filename, reference_filename, minimum_psnr) \
    _ASSERT_VIDEO_QUALITY7(filename, reference_filename, minimum_psnr, -1, -1, -1, -1)

#define _EXPECT_VIDEO_QUALITY3(filename, reference_filename, minimum_psnr) \
    _EXPECT_VIDEO_QUALITY7(filename, reference_filename, minimum_psnr, -1, -1, -1, -1)

#define _ASSERT_VIDEO_QUALITY7(filename, reference_filename, minimum_psnr, \
                               reference_left, reference_top, reference_right, reference_bottom) \
    _ASSERT_VIDEO_QUALITY11(filename, reference_filename, minimum_psnr, \
                            reference_left, reference_top, reference_right, reference_bottom, -1, -1, -1, -1)

#define _EXPECT_VIDEO_QUALITY7(filename, reference_filename, minimum_psnr, \
                               reference_left, reference_top, reference_right, reference_bottom) \
    _EXPECT_VIDEO_QUALITY11(filename, reference_filename, minimum_psnr, \
                            reference_left, reference_top, reference_right, reference_bottom, -1, -1, -1, -1)

#define _ASSERT_VIDEO_QUALITY11(filename, reference_filename, minimum_psnr, \
                                reference_left, reference_top, reference_right, reference_bottom, \
                                source_left, source_top, source_right, source_bottom) { \
    auto command = _QUALITY_COMMAND(filename, reference_filename, minimum_psnr, \
                                    reference_left, reference_top, reference_right, reference_bottom, \
                                    source_left, source_top, source_right, source_bottom); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected video quality (expected >=" << (minimum_psnr) << ')'; }

#define _EXPECT_VIDEO_QUALITY11(filename, reference_filename, minimum_psnr, \
                                reference_left, reference_top, reference_right, reference_bottom, \
                                source_left, source_top, source_right, source_bottom) { \
    auto command = _QUALITY_COMMAND(filename, reference_filename, minimum_psnr, \
                                    reference_left, reference_top, reference_right, reference_bottom, \
                                    source_left, source_top, source_right, source_bottom); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected video quality (expected >=" << (minimum_psnr) << ')'; }

////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_RGB_THRESHOLD 8

#define _RGB_COMMAND(filename, red, blue, green, threshold) \
     (std::string("resources/assert-rgb.py ") + \
                     (filename) + ' ' + \
                     std::to_string(red) + ',' + std::to_string(blue) + ',' + std::to_string(green) + ' ' + \
                     std::to_string(threshold))

#define ASSERT_VIDEO_RGB(filename, red, blue, green, threshold) { \
    auto command = _RGB_COMMAND(filename, red, blue, green, threshold); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected RGB variation (expected <=" << (threshold) << ')'; }

#define EXPECT_VIDEO_RGB(filename, red, blue, green, threshold) { \
    auto command = _RGB_COMMAND(filename, red, blue, green, threshold); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected RGB variation (expected <=" << (threshold) << ')'; }

#define ASSERT_VIDEO_RED(filename) ASSERT_VIDEO_RGB(filename, 255, 0, 0, DEFAULT_RGB_THRESHOLD)
#define ASSERT_VIDEO_GREEN(filename) ASSERT_VIDEO_RGB(filename, 0, 255, 0, DEFAULT_RGB_THRESHOLD)
#define ASSERT_VIDEO_BLUE(filename) ASSERT_VIDEO_RGB(filename, 0, 0, 255, DEFAULT_RGB_THRESHOLD)

#define EXPECT_VIDEO_RED(filename) EXPECT_VIDEO_RGB(filename, 255, 0, 0, DEFAULT_RGB_THRESHOLD)
#define EXPECT_VIDEO_GREEN(filename) EXPECT_VIDEO_RGB(filename, 0, 255, 0, DEFAULT_RGB_THRESHOLD)
#define EXPECT_VIDEO_BLUE(filename) EXPECT_VIDEO_RGB(filename, 0, 0, 255, DEFAULT_RGB_THRESHOLD)

////////////////////////////////////////////////////////////////////////////////////////////////////

CudaFrame CREATE_BLACK_FRAME(const Configuration &);
void ASSERT_BLACK_FRAME(const DecodedFrame &);
std::string TRANSCODE_RAW_TO_H264(const std::string& filename, size_t height, size_t width, size_t framerate);
size_t COUNT_FRAMES(const std::string &filename, size_t stream_index=0u);

#endif //LIGHTDB_ASSERTVIDEO_H
