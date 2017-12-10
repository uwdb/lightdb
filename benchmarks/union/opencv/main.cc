#include <cstdio>
#include "opencv2/opencv.hpp"

using namespace cv;

#define OUTPUT_FILENAME std::string("out.mp4")

// video Union function - stacks the same video next to each other
void vUnion(const char* input_filename){

  VideoCapture input(input_filename);
  auto frames = std::lround(input.get(CAP_PROP_FRAME_COUNT));
  auto fourcc = std::lround(input.get(CV_CAP_PROP_FOURCC));
  auto fps = std::lround(input.get(CAP_PROP_FPS));

  auto width = std::lround(input.get(CAP_PROP_FRAME_WIDTH));
  auto height = std::lround(input.get(CAP_PROP_FRAME_HEIGHT));

  Size size{width, height};
  Mat frame{size, CV_8UC3};
  Size out_size{2*width, height};
  Mat out_frame{out_size, CV_8UC3};
  VideoWriter writer(OUTPUT_FILENAME, 0x21, fps, out_size, true);

  if(!input.isOpened())
      throw std::runtime_error("Failed to open input.");

  while(frames--) {
      input >> frame; // get a new frame from camera

      for (auto i = 0; i < 2; i++){
        int offset_x = i * size.width;
        auto range_x = Range(offset_x, offset_x + size.width),
        	range_y = Range(0, 0 + size.height);
        frame.copyTo(out_frame(Range::all(), Range(i*frame.cols,i*frame.cols + frame.cols)));
      }

      writer.write(out_frame);
  }
}

void overlay(const char* input_filename, const char *input_filename2){

  VideoCapture input(input_filename), input2(input_filename2);
  auto frames = std::lround(input.get(CAP_PROP_FRAME_COUNT));
  auto fourcc = std::lround(input.get(CV_CAP_PROP_FOURCC));
  auto fps = std::lround(input.get(CAP_PROP_FPS));

  auto width = std::lround(input.get(CAP_PROP_FRAME_WIDTH));
  auto height = std::lround(input.get(CAP_PROP_FRAME_HEIGHT));

  Size size{width, height};
  Mat frame{size, CV_8UC3};
  Mat frame2{size, CV_8UC3};
  Size out_size{width, height};
  Mat out_frame{out_size, CV_8UC3};
  VideoWriter writer(OUTPUT_FILENAME, 0x21, fps, out_size, true);

  if(!input.isOpened())
      throw std::runtime_error("Failed to open input.");

  while(frames--) {
      input >> frame; // get a new frame from camera
      input2 >> frame2;

      addWeighted(frame, 255.0, frame2, 255.0, 0.0, out_frame);

      writer.write(out_frame);
  }
}

int main(int argc, char** argv) {
    if(argc < 3)
        printf("Usage: %s [overlay,stack] [input filename]\n", argv[0]);
    else if(std::string(argv[1]) == "stack") {
	vUnion(argv[2]);
    } else if(std::string(argv[1]) == "overlay") {
        overlay(argv[2], argv[3]);
    }
}
