#ifndef LIGHTDB_TESTRESOURCES_H
#define LIGHTDB_TESTRESOURCES_H

#include "number.h"

struct {
    struct {
        const std::string name = "red10";
        const size_t height = 240;
        const size_t width = 320;
        const size_t frames = 250;
        const size_t framerate = 25;
    } red10;

    struct {
        const std::string name = "green10";
        const size_t height = 240;
        const size_t width = 320;
        const size_t frames = 250;
        const size_t framerate = 25;
    } green10;

    struct {
        struct {
            const std::string name = "resources/black.h264";
            const unsigned int height = 2160;
            const unsigned int width = 3840;
        } black;

    } videos;

    struct {
        const char *hevc = "out.hevc";
        const char *h264 = "out.h264";
        const char *raw = "out.raw";
    } out;

    const std::string catalog_name = "resources";
} Resources;

#endif //LIGHTDB_TESTRESOURCES_H
