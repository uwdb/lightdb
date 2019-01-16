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

    const std::string catalog_name = "resources";
} Resources;

#endif //LIGHTDB_TESTRESOURCES_H
