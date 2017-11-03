#ifndef VISUALCLOUD_ASSERT_VIDEO_H
#define VISUALCLOUD_ASSERT_VIDEO_H

#include "gtest/gtest.h"

#define _VALID_VIDEO_COMMAND(filename) \
     (std::string("ffprobe -hide_banner -loglevel quiet ") + (filename))
#define _FRAME_COMMAND(filename, frame_count) \
     (std::string("resources/assert-frames.sh ") + \
                     (filename) + ' ' + \
                     std::to_string(frame_count))
#define _RESOLUTION_COMMAND(filename, height, width) \
     (std::string("resources/assert-resolution.sh ") + \
                     (filename) + ' ' + \
                     std::to_string(height) + ' ' + \
                     std::to_string(width))
#define _QUALITY_COMMAND(filename, reference_filename, minimum_psnr) \
     (std::string("resources/assert-quality.sh ") + \
                     (filename) + ' ' + \
                     (reference_filename) + ' ' + \
                     std::to_string(minimum_psnr))

#define ASSERT_VIDEO_FRAMES(filename, frame_count) { \
    auto command = _FRAME_COMMAND(filename, frame_count); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": expected " << (frame_count) << " frames"; }

#define EXPECT_VIDEO_FRAMES(filename, frame_count) { \
    auto command = _FRAME_COMMAND(filename, frame_count); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": expected " << (frame_count) << " frames"; }

#define ASSERT_VIDEO_VALID(filename) { \
    auto command = _VALID_VIDEO_COMMAND(filename); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": video could not be parsed"; }

#define EXPECT_VIDEO_VALID(filename) { \
    auto command = _VALID_VIDEO_COMMAND(filename); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": video could not be parsed"; }

#define ASSERT_VIDEO_RESOLUTION(filename, height, width) { \
    auto command = _RESOLUTION_COMMAND(filename, height, width); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected video resolution (expected " << (width) << 'x' << (height) << ')'; }

#define EXPECT_VIDEO_RESOLUTION(filename, height, width) { \
    auto command = _RESOLUTION_COMMAND(filename, height, width); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected video resolution (expected " << (width) << 'x' << (height) << ')'; }

#define ASSERT_VIDEO_QUALITY(filename, reference_filename, minimum_psnr) { \
    auto command = _QUALITY_COMMAND(filename, reference_filename, minimum_psnr); \
    ASSERT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected video quality (expected >=" << (minimum_psnr) << ')'; }

#define EXPECT_VIDEO_QUALITY(filename, reference_filename, minimum_psnr) { \
    auto command = _QUALITY_COMMAND(filename, reference_filename, minimum_psnr); \
    EXPECT_EQ(system(command.c_str()), 0) \
        << (filename) << ": unexpected video quality (expected >=" << (minimum_psnr) << ')'; }

#endif //VISUALCLOUD_ASSERT_VIDEO_H
