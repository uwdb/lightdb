#include <cstdio>
#include "opencv2/opencv.hpp"
#include "opencv2/cudaarithm.hpp"
#include "opencv2/cudafilters.hpp"

using namespace cv;

#define OUTPUT_FILENAME std::string("out.mp4")

// applies grayscale conversion to input image
// using opencv-provided cvtColor
void map_gray(const char* input_filename){
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

// applies gaussian blur convoluation to input image
void map_blur(const char* input_filename){

  VideoCapture input(input_filename);
  auto frames = std::lround(input.get(CAP_PROP_FRAME_COUNT)) - 1;
  auto fourcc = std::lround(input.get(CV_CAP_PROP_FOURCC));
  auto fps = std::lround(input.get(CAP_PROP_FPS));

  auto width = std::lround(input.get(CAP_PROP_FRAME_WIDTH));
  auto height = std::lround(input.get(CAP_PROP_FRAME_HEIGHT));

  Mat frame, blurred;
  cv::cuda::GpuMat gpuFrame, gpuBlurred;
  Size size{width, height};
  VideoWriter writer(OUTPUT_FILENAME, 0x21, fps, size, false);
  //auto filter = cv::cuda::createGaussianFilter(gpuFrame.type(), gpuBlurred.type(), {5, 5}, 0);

  if(!input.isOpened())
      throw std::runtime_error("Failed to open input.");

  while(frames--) {
      input >> frame; // get a new frame from camera
      gpuFrame.upload(frame);
      auto filter = cv::cuda::createGaussianFilter(gpuFrame.type(), gpuBlurred.type(), {5, 5}, 0);
      filter->apply(gpuFrame, gpuBlurred);
      //cv::cuda::blur(gpuFrame, gpuBlurred, {5, 5});

      //cv::Mat result(gpuBlurred);
      gpuBlurred.download(blurred);
      //printf("%d\n", frames);
      //cvtColor(frame, blurred, COLOR_BGR2GRAY);

      writer.write(blurred);
  }
}

int main(int argc, char** argv) {
    if(argc != 3)
        printf("Usage: %s [input filename] [gray,blur]\n", argv[0]);
    else if(std::string(argv[2]) == "gray") {
	map_gray(argv[1]);
    } else if(std::string(argv[2]) == "blur") {
        map_blur(argv[1]);
    }
}
