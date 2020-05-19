#include <cstdio>
#include "opencv2/opencv.hpp"
#include <opencv2/cudacodec.hpp>
#include "opencv2/cudaarithm.hpp"
#include "opencv2/cudafilters.hpp"

using namespace cv;

#define TILE_FILENAME(i) (std::string("tile") + std::to_string(i) + ".mp4")
#define OUTPUT_FILENAME(i) (std::string("out") + std::to_string(static_cast<unsigned int>(i)) + ".mp4")

void tile(const char* input_filename,
          unsigned int rows, unsigned int columns,
          std::vector<size_t> bitrates)
{
    printf("Open reader\n");
    cv::Ptr<cv::cudacodec::VideoReader> input = cv::cudacodec::createVideoReader(std::string(input_filename));
    auto frames = std::lround(VideoCapture("model.mp4").get(CAP_PROP_FRAME_COUNT));
    auto fourcc = std::lround(VideoCapture("model.mp4").get(CV_CAP_PROP_FOURCC));
    auto fps = std::lround(VideoCapture("model.mp4").get(CAP_PROP_FPS));
    cv::cuda::GpuMat frame;
    Mat hosttile;
    Size size{std::lround(VideoCapture("model.mp4").get(CAP_PROP_FRAME_WIDTH))/columns,
              std::lround(VideoCapture("model.mp4").get(CAP_PROP_FRAME_HEIGHT)/rows)};
    std::vector<VideoWriter> writers;
    //std::vector<cv::Ptr<cv::cudacodec::VideoWriter>> writers; GPU writer not supported on Linux

    printf("Open writers\n");
    for(auto i = 0u; i < rows * columns; i++) {
        //cv::cudacodec::EncoderParams parameters{};
        //parameters.AvgBitrate = bitrates.at(i)*10000;
        //writers.push_back(cv::cudacodec::createVideoWriter(TILE_FILENAME(i), size,  fps, parameters));
        writers.push_back({TILE_FILENAME(i), fourcc, fps, size, true, bitrates.at(i)*1000});
    }

    printf("Tiling %d frames\n", frames);
    if(frames < 0)
        exit(1);

    while(frames--) {
        //input >> frame;
        input->nextFrame(frame); // get a new frame from camera

        if(!frame.empty())
            for(auto i = 0u; i < columns; i++)
                for(auto j = 0u; j < rows; j++) {
                    auto offset_x = i * size.width,
                         offset_y = j * size.height;
                    auto range_x = Range(offset_x, offset_x + size.width),
                         range_y = Range(offset_y, offset_y + size.height);
                    auto tile = frame(range_y, range_x);
                    tile.download(hosttile);

                    writers.at(i + j * columns).write(hosttile);
                }
    }
}

void join(unsigned int rows, unsigned int columns, unsigned int bitrate)
{
    std::vector<VideoCapture> inputs;

    for(auto i = 0u; i < rows * columns; i++) {
        inputs.push_back({TILE_FILENAME(i)});
        if(!inputs.at(i).isOpened())
            throw std::runtime_error("Failed to open input.");
    }

    auto frames = std::lround(inputs.at(0).get(CAP_PROP_FRAME_COUNT));
    auto fourcc = std::lround(inputs.at(0).get(CV_CAP_PROP_FOURCC));
    auto fps = std::lround(inputs.at(0).get(CAP_PROP_FPS));
    Size tilesize{std::lround(inputs.at(0).get(CAP_PROP_FRAME_WIDTH)),
              std::lround(inputs.at(0).get(CAP_PROP_FRAME_HEIGHT))};
    Size outsize{std::lround(inputs.at(0).get(CAP_PROP_FRAME_WIDTH))*columns,
              std::lround(inputs.at(0).get(CAP_PROP_FRAME_HEIGHT)*rows)};
    Mat frame, outframe{outsize,
                        CV_8UC3};
    VideoWriter writer{OUTPUT_FILENAME(0), fourcc, fps, outsize, true, bitrate*1000};
    auto frames_written = 0u;

    printf("Joining\n");

    while(frames--) {
        std::vector<Mat> tiles;
        tiles.reserve(rows * columns);
        for(auto &input: inputs) {
            input >> frame;
            tiles.push_back(frame.clone());
        }

        if(!tiles.at(0).empty()) {
            for (auto i = 0; i < columns; i++)
                for (auto j = 0; j < rows; j++) {
                    auto offset_x = i * tilesize.width,
                            offset_y = j * tilesize.height;
                    auto range_x = Range(offset_x, offset_x + tilesize.width),
                            range_y = Range(offset_y, offset_y + tilesize.height);
                    auto target = outframe(range_y, range_x);
                    tiles.at(i + j * columns).copyTo(target);
                }

            writer.write(outframe);

            if(++frames_written % fps == 0) {
                writer.release();
                writer.open(OUTPUT_FILENAME(frames_written / fps), fourcc, fps, outsize, true, bitrate * 1000);
            }
        }
    }

    for(auto i = 0u; i < rows * columns; i++) {
        remove(TILE_FILENAME(i).c_str());
    }
}

int main(int argc, char** argv) {
    if(argc != 2)
        printf("Usage: %s [input filename]\n", argv[0]);
    else {
        auto rows = 4u, columns = 4u;
        tile(argv[1], rows, columns, {50, 50, 50, 50, 50, 50, 1000, 5000, 1000, 50, 50, 50, 50, 50, 50, 50});
        join(rows, columns, 5000);
    }
}
