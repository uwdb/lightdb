#ifndef VISUALCLOUD_ASSERT_VIDEO_H
#define VISUALCLOUD_ASSERT_VIDEO_H

#include "Frame.h"
#include "gtest/gtest.h"

#define DEFAULT_PSNR 30

////////////////////////////////////////////////////////////////////////////////////////////////////

#define _FRAME_COMMAND(filename, frame_count) \
     (std::string("resources/assert-frames.sh ") + \
                     (filename) + ' ' + \
                     std::to_string(frame_count))

#define ASSERT_VIDEO_FRAMES(filename, frame_count) { \
    auto command = _FRAME_COMMAND(filename, frame_count); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": expected " << (frame_count) << " frames"; }

#define EXPECT_VIDEO_FRAMES(filename, frame_count) { \
    auto command = _FRAME_COMMAND(filename, frame_count); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": expected " << (frame_count) << " frames"; }

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
                     std::to_string(height) + ' ' + \
                     std::to_string(width))

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

#define _RGB_COMMAND(filename, reference_filename, threshold) \
     (std::string("resources/assert-rgb.py ") + \
                     (filename) + ' ' + \
                     (reference_filename) + ' ' + \
                     std::to_string(threshold))

#define ASSERT_VIDEO_MEAN_RGB(filename, reference_filename, threshold) { \
    auto command = _RGB_COMMAND(filename, reference_filename, threshold); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected mean RGB difference (expected >=" << (threshold) << ')'; }

#define EXPECT_VIDEO_MEAN_RGB(filename, reference_filename, threshold) { \
    auto command = _RGB_COMMAND(filename, reference_filename, threshold); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected mean RGB difference (expected >=" << (threshold) << ')'; }

////////////////////////////////////////////////////////////////////////////////////////////////////

Frame CREATE_BLACK_FRAME(const Configuration &);
void ASSERT_BLACK_FRAME(const Frame &);

#endif //VISUALCLOUD_ASSERT_VIDEO_H
