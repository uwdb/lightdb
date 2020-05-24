#include <cstdio>
#include "opencv2/opencv.hpp"

using namespace cv;

#define OUTPUT_FILENAME std::string("out.mp4")

// output_fps determines sample rate of VideoCapture
// width, height, offset_x, offset_y do volumetric select
void select(const char* input_filename,
          unsigned int width, unsigned int height,
          unsigned int offset_x, unsigned int offset_y,
          unsigned int output_fps){

  VideoCapture input(input_filename);
  auto frames = std::lround(input.get(CAP_PROP_FRAME_COUNT));
  auto fourcc = std::lround(input.get(CV_CAP_PROP_FOURCC));
  auto fps = std::lround(input.get(CAP_PROP_FPS));

  if ((width == -1) && (height == -1) &&
      (offset_x == -1) && (offset_y == -1)){
        width = std::lround(input.get(CAP_PROP_FRAME_WIDTH));
        height = std::lround(input.get(CAP_PROP_FRAME_WIDTH));
        offset_x = 0;
        offset_y = 0;
      }
  else if (output_fps == -1){
    output_fps = fps;
  }

  Mat frame;
  Size size{width, height};
  VideoWriter writer(OUTPUT_FILENAME, 0x21, output_fps, size, true);

  if(!input.isOpened())
      throw std::runtime_error("Failed to open input.");

  while(frames--) {
      input >> frame; // get a new frame from camera

      if(!frame.empty()){
        auto range_x = Range(offset_x, offset_x + size.width),
             range_y = Range(offset_y, offset_y + size.height);
        auto tile = frame(range_y, range_x);
        writer.write(tile);
      }
  }
}

void select_time(const char* input_filename,
          unsigned int width, unsigned int height,
          unsigned int start, unsigned int end) {

  VideoCapture input(input_filename);
  auto frames = std::lround(input.get(CAP_PROP_FRAME_COUNT));
  auto fourcc = std::lround(input.get(CV_CAP_PROP_FOURCC));
  auto fps = std::lround(input.get(CAP_PROP_FPS));

  Mat frame;
  Size size{width, height};
  VideoWriter writer(OUTPUT_FILENAME, 0x21, fps, size, true);

  if(!input.isOpened())
      throw std::runtime_error("Failed to open input.");

  for(int i = 0; i < fps * start; i++)
     input >> frame;

  frames = (end - start) * fps;
  while(frames--) {
      input >> frame; // get a new frame from camera

      if(!frame.empty()){
        writer.write(frame);
      }
  }
}

void select_tile(const char* input_filename,
          unsigned int frames,
          unsigned int rows, unsigned int cols,
          unsigned int row, unsigned int col)
{
  VideoCapture input(input_filename);
  //auto frames = std::lround(input.get(CAP_PROP_FRAME_COUNT));
  auto fourcc = std::lround(input.get(CV_CAP_PROP_FOURCC));
  auto fps = std::lround(input.get(CAP_PROP_FPS));

  auto width = std::lround(input.get(CAP_PROP_FRAME_WIDTH))/cols;
  auto height = std::lround(input.get(CAP_PROP_FRAME_WIDTH))/rows;
  auto left = col * width;
  auto top = row * height;

  Mat frame;
  Size size{width, height};
  VideoWriter writer(OUTPUT_FILENAME, 0x21, fps, size, true);

  if(!input.isOpened())
      throw std::runtime_error("Failed to open input.");

  while(frames--) {
      input >> frame; // get a new frame from camera

      if(!frame.empty()){
        auto range_x = Range(left, size.width),
             range_y = Range(top, size.height);
        auto tile = frame(range_y, range_x);
        writer.write(tile);
      }
  }
}

int main(int argc, char** argv) {
    if(argc != 7 && argc != 8) {
        printf("Usage: %s crop [input filename] crop width height left top\n", argv[0]);
        printf("          time [input filename] time width height start end\n");
        printf("          tile [input filename] rows cols row col\n");
    } else if(std::string(argv[1]) == "crop") {
	select(argv[2], atoi(argv[3]), atoi(argv[4]),
		atoi(argv[5]), atoi(argv[6]), -1);
    } else if(std::string(argv[1]) == "time") {
        select_time(argv[2], atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    } else if(std::string(argv[1]) == "tile") {
        select_tile(argv[2], atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
    }
}
