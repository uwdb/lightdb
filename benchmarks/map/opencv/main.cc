#include <cstdio>
#include "opencv2/opencv.hpp"

using namespace cv;

#define OUTPUT_FILENAME std::string("out.mp4")

// applies grayscale conversion to input image
// using opencv-provided cvtColor
void map(const char* input_filename){
  
  VideoCapture input(input_filename);
  auto frames = std::lround(input.get(CAP_PROP_FRAME_COUNT));
  auto fourcc = std::lround(input.get(CV_CAP_PROP_FOURCC));
  auto fps = std::lround(input.get(CAP_PROP_FPS));
 
  auto width = std::lround(input.get(CAP_PROP_FRAME_WIDTH));
  auto height = std::lround(input.get(CAP_PROP_FRAME_HEIGHT));

  Mat frame, gray;
  Size size{width, height};
  VideoWriter writer(OUTPUT_FILENAME, 0x21, fps, size, false);
  
  if(!input.isOpened())
      throw std::runtime_error("Failed to open input.");

  while(frames--) {
      input >> frame; // get a new frame from camera
      cvtColor(frame, gray, COLOR_BGR2GRAY);

      writer.write(gray);
  }
}

int main(int argc, char** argv) {
    if(argc != 2)
        printf("Usage: %s [input filename]\n", argv[0]);
    else {
	map(argv[1]);
    }
}
