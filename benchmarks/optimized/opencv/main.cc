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

int main(int argc, char** argv) {
    if(argc != 6)
        printf("Usage: %s [input filename]\n", argv[0]);
    else {
	select(argv[1], atoi(argv[2]), atoi(argv[3]), 
		atoi(argv[4]), atoi(argv[5]), -1);
    }
}
