#include <cstdio>
#include "darknet.h"
#include "opencv2/opencv.hpp"
#include "opencv2/cudaarithm.hpp"
#include "opencv2/cudafilters.hpp"

using namespace cv;

#define OUTPUT_FILENAME std::string("out.mp4")

float **make_probs(network *net); //BH


// Adapted from image load_image_stb(char *filename, int channels) in darknet
image make_darknet_image(const Mat frame, const size_t width, const size_t height)
{
    int w=width, h=height, c = 3;

    int i,j,k;
    image im = make_image(w, h, c);
    for(k = 0; k < c; ++k){
        for(j = 0; j < h; ++j){
            for(i = 0; i < w; ++i){
                int dst_index = i + w*j + w*h*k;
                auto rgb = frame.at<Vec3b>(Point(i, j));
                im.data[dst_index] = (float)rgb[k]/255.;
            }
        }
    }
    return im;
}

Mat &update_frame(Mat &frame, const image &image, const size_t width, const size_t height)
{
    int w=width, h=height, c = 3;

    int i,j;
        for(j = 0; j < h; ++j){
            for(i = 0; i < w; ++i){
                int dst_index0 = i + w*j + w*h*0;
                int dst_index1 = i + w*j + w*h*1;
                int dst_index2 = i + w*j + w*h*2;
                frame.at<Vec3b>(Point(i, j)) = Vec3b{image.data[dst_index1] * 255, image.data[dst_index1] * 255, image.data[dst_index2] * 255};
            }
        }
    return frame;
}

void map_yolo(const char* input_filename){
  VideoCapture input(input_filename);
  auto frames = std::lround(input.get(CAP_PROP_FRAME_COUNT));
  auto fourcc = std::lround(input.get(CV_CAP_PROP_FOURCC));
  auto fps = std::lround(input.get(CAP_PROP_FPS));

  auto width = std::lround(input.get(CAP_PROP_FRAME_WIDTH));
  auto height = std::lround(input.get(CAP_PROP_FRAME_HEIGHT));

  Mat frame;
  Size size{width, height};
  VideoWriter writer(OUTPUT_FILENAME, 0x21, fps, size, false);

  if(!input.isOpened())
      throw std::runtime_error("Failed to open input.");

  auto network = load_network("/home/bhaynes/projects/darknet/cfg/tiny-yolo.cfg", "/home/bhaynes/projects/darknet/tiny-yolo.weights", 0);
  auto metadata = get_metadata("/home/bhaynes/projects/darknet/cfg/coco.data");
  auto num = num_boxes(network);
  auto *boxes = make_boxes(network);
  auto **probs = make_probs(network);
  auto threshold = 0.5f, hier_thresh=0.5f, nms=.45f;

  while(frames--) {
    input >> frame; // get a new frame from camera

    auto image = make_darknet_image(frame, width, height);
    network_detect(network, image, threshold, hier_thresh, nms, boxes, probs);

    //update_frame(frame, image, width, height);

    writer.write(frame);
  }
}

int main(int argc, char** argv) {
    if(argc != 2)
        printf("Usage: %s [input filename]\n", argv[0]);
    else {
	map_yolo(argv[1]);
    }
}
