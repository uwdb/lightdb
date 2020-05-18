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

void time_partition(const char* input_filename, const float delta_time)
{
    VideoCapture input(input_filename);
    auto frames = std::lround(input.get(CAP_PROP_FRAME_COUNT));
    auto fourcc = std::lround(input.get(CV_CAP_PROP_FOURCC));
    auto fps = std::lround(input.get(CAP_PROP_FPS));
    Mat frame;
    Size size{input.get(CAP_PROP_FRAME_WIDTH),
              input.get(CAP_PROP_FRAME_HEIGHT)};
    auto index = 0u;
    VideoWriter writer{TILE_FILENAME(index++), fourcc, fps, size, true};
    auto delta_frames = delta_time * fps;

    if(!input.isOpened())
        throw std::runtime_error("Failed to open input.");

    printf("Partitioning into %.02f second chunks\n", delta_time);

    while(frames--) {
        input >> frame; // get a new frame from camera

        if(!frame.empty())
		{
		writer.write(frame);
                if(delta_frames-- == 0)
			{
			writer.release();
			writer = VideoWriter{TILE_FILENAME(index++), fourcc, fps, size, true};
			delta_frames = delta_time * fps;
			}
                }
    }
}

int main(int argc, char** argv) {
    if(argc != 5 && argc != 4)
        printf("Usage: %s [tile,time] [input filename] [rows] [columns]\n", argv[0]);
    else if(std::string(argv[1]) == "tile") {
        auto rows = atoi(argv[3]), columns = atoi(argv[4]);
        tile(argv[2], rows, columns);
    }
    else if(std::string(argv[1]) == "time") {
        auto delta_time = atof(argv[3]);
        time_partition(argv[2], delta_time);
    }
}
