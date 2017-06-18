#ifndef _TILER_H
#define _TILER_H

#include "TileVideoEncoder.h"
#include <string>
#include <vector>

int ExecuteTiler(std::vector<EncodeConfig> &, const TileDimensions);

int ExecuteTiler(const std::string inputFilename, const std::string outputFilenameFormat, unsigned int height,
                 unsigned int width, size_t tileRows, size_t tileColumns, unsigned int codec, std::string preset,
                 unsigned int fps, unsigned int gop_length, size_t bitrate, unsigned int rcmode, unsigned int deviceId);

EncodeConfig MakeTilerConfiguration(char *inputFilename, char *outputFilenameFormat, const unsigned int height,
                                    const unsigned int width, const size_t tileRows, const size_t tileColumns,
                                    const unsigned int codec, char *preset, const unsigned int fps,
                                    const unsigned int gop_length, const size_t bitrate, const unsigned int rcmode,
                                    const unsigned int deviceId);

#endif