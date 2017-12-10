#include <cstdio>
#include "opencv2/opencv.hpp"

using namespace cv;

#define TILE_FILENAME(i) (std::string("tile") + std::to_string(i) + ".mp4")
#define OUTPUT_FILENAME(i) (std::string("out") + std::to_string(static_cast<unsigned int>(i)) + ".mp4")

void tile(const char* input_filename,
          unsigned int rows, unsigned int columns)
{
    VideoCapture input(input_filename);
    auto frames = std::lround(input.get(CAP_PROP_FRAME_COUNT));
    auto fourcc = std::lround(input.get(CV_CAP_PROP_FOURCC));
    auto fps = std::lround(input.get(CAP_PROP_FPS));
    Mat frame;
    Size size{std::lround(input.get(CAP_PROP_FRAME_WIDTH))/columns,
              std::lround(input.get(CAP_PROP_FRAME_HEIGHT)/rows)};
    std::vector<VideoWriter> writers;

    if(!input.isOpened())
        throw std::runtime_error("Failed to open input.");

    for(auto i = 0u; i < rows * columns; i++)
        writers.push_back({TILE_FILENAME(i), fourcc, fps, size, true});

    printf("Partitioning\n");

    while(frames--) {
        input >> frame; // get a new frame from camera

        if(!frame.empty())
            for(auto i = 0u; i < columns; i++)
                for(auto j = 0u; j < rows; j++) {
                    auto offset_x = i * size.width,
                         offset_y = j * size.height;
                    auto range_x = Range(offset_x, offset_x + size.width),
                         range_y = Range(offset_y, offset_y + size.height);
                    auto tile = frame(range_y, range_x);

                    writers.at(i + j * columns).write(tile);
                }
    }
}

int main(int argc, char** argv) {
    if(argc != 4)
        printf("Usage: %s [input filename] [rows] [columns]\n", argv[0]);
    else {
        auto rows = atoi(argv[2]), columns = atoi(argv[3]);
        tile(argv[1], rows, columns);
    }
}
