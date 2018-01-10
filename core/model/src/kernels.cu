#include "stdio.h"

extern "C"
__global__
void blur(unsigned char* input, unsigned char* output, const unsigned int width, const unsigned int height) {
    const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
    int x = index % width;
    int y = (index-x)/width;
    int size = 3;

    if(index < width * height) {
        unsigned int output_red = 0,
                     output_green = 0,
                     output_blue = 0;

        int applications = 0;
        for(int delta_x = -size; delta_x < size+1; ++delta_x) {
            for(int delta_y = -size; delta_y < size+1; ++delta_y) {
                if(x + delta_x >= 0 && x + delta_x < width && y + delta_y >= 0 && y + delta_y < height) {
                    const int currentIndex = (index+delta_x+delta_y*width)*3;
                    output_red += input[currentIndex];
                    output_green += input[currentIndex+1];
                    output_blue += input[currentIndex+2];
                    applications++;
                }
            }
        }

        output[index*3] = output_red / applications;
        output[index*3+1] = output_green / applications;
        output[index*3+2] = output_blue / applications;
    }
}

extern "C"
__global__
void overlay(unsigned char* left, unsigned char* right, unsigned char* output,
             const unsigned int width, const unsigned int height,
             const unsigned int transparent_color) {
    const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

    if(right[index] != transparent_color)
        left[index] = right[index];
}

extern "C"
__global__
void resize(const unsigned int* input, unsigned int* output,
            const unsigned int input_width, const unsigned int input_height,
            const unsigned int output_width, const unsigned int output_height,
            const float fx, const float fy) {
    const unsigned int output_x = blockDim.x * blockIdx.x + threadIdx.x;
    const unsigned int output_y = blockDim.y * blockIdx.y + threadIdx.y;
    const unsigned int output_index = output_y * output_width + output_x;

    if (output_x < output_width && output_y < output_height)
    {
        const unsigned int src_x = output_x / fx;
        const unsigned int src_y = output_y / fy;
        const unsigned int input_index = src_y * input_width + src_x;

        output[output_index] = input[input_index];
    }
}

extern "C"
__global__
void resize_weighted(
            const unsigned int* input, float* output,
            const unsigned int input_width, const unsigned int input_height,
            const unsigned int output_width, const unsigned int output_height,
            const float fx, const float fy) {
    const unsigned int output_x = blockDim.x * blockIdx.x + threadIdx.x;
    const unsigned int output_y = blockDim.y * blockIdx.y + threadIdx.y;
    const unsigned int output_index = output_y * output_width + output_x;

    if (output_x < output_width && output_y < output_height)
    {
        const unsigned int src_x = output_x / fx;
        const unsigned int src_y = output_y / fy;
        const unsigned int input_index = src_y * input_width + src_x;

        const unsigned int rgba = input[input_index];
        const unsigned int channel_size = output_height * output_width;

        output[output_index] =                  (rgba & 0xff) / 255.0f;
        output[output_index + channel_size] =   ((rgba >> 8) & 0xff) / 255.0f;
        output[output_index + 2*channel_size] = ((rgba >> 16) & 0xff) / 255.0f;
    }
}

/*extern "C"
__global__
void extract_luma_and_resize(
            const unsigned int* input, unsigned int* output,
            const unsigned int input_width, const unsigned int input_height,
            const unsigned int output_width, const unsigned int output_height,
            const float fx, const float fy) {
    const unsigned int output_x = blockDim.x * blockIdx.x + threadIdx.x;
    const unsigned int output_y = blockDim.y * blockIdx.y + threadIdx.y;
    const unsigned int output_index = output_y * output_width + output_x;

    if (output_x < output_width && output_y < output_height)
    {
        const unsigned int src_x = output_x / fx;
        const unsigned int src_y = output_y / fy;
        const unsigned int input_index = src_y * input_width + src_x;

        output[output_index] = input[input_index];
    }
}*/

typedef struct {
    int classes;
    char **names;
} metadata;
typedef struct {
    float x, y, w, h;
} box;


extern "C"
__global__
void draw_detections(
            unsigned char* nv12image,
            const unsigned int pitch,
            const unsigned int width,
            const unsigned int height,
            const float *probabilities,
            const unsigned int class_count,
            const box *boxes,
            const unsigned int box_count,
            const float fx, const float fy) {
    const unsigned int prediction_class = blockDim.x * blockIdx.x + threadIdx.x;
    const unsigned int box_id = blockDim.y * blockIdx.y + threadIdx.y;

    const unsigned int prediction_index = box_id * class_count + prediction_class;

    if(prediction_class < class_count &&
            box_id < box_count &&
            probabilities[prediction_index] > 0.001)
    {

        const box &b = boxes[box_id];

        const unsigned int x1 = (b.x - b.w/2) / fx;
        const unsigned int x2 = (b.x + b.w/2) / fx;
        const unsigned int y1 = (b.y - b.h/2) / fy;
        const unsigned int y2 = (b.y + b.h/2) / fy;
        const unsigned int image_x1 = min(x1, width - 3);
        const unsigned int image_x2 = min(x2, width - 3);
        const unsigned int image_y1 = min(y1, height - 3);
        const unsigned int image_y2 = min(y2, height - 3);

        nv12image[image_y1 * pitch + image_x1] = 0;
        nv12image[image_y2 * pitch + image_x2] = 0;
        nv12image[image_y1 * pitch + image_x1] = 0;
        nv12image[image_y2 * pitch + image_x2] = 0;

/*        for(int y = image_y1; y < image_y2; y++)
        {
            nv12image[y * pitch + image_x1] = 0;
            nv12image[(y+1) * pitch + image_x1] = 0;
            nv12image[(y+2) * pitch + image_x1] = 0;
        }
        for(int y = image_y1; y < image_y2; y++)
        {
            nv12image[y * pitch + image_x2] = 0;
            nv12image[(y+1) * pitch + image_x2 + 1] = 0;
            nv12image[(y+2) * pitch + image_x2 + 2] = 0;
        }
        for(int x = image_x1; x < image_x2; x++)
        {
            nv12image[image_y1 * pitch + x] = 0;
            nv12image[image_y1 * pitch + x + 1] = 0;
            nv12image[image_y1 * pitch + x + 2] = 0;
        }
        for(int x = image_x1; x < image_x2; x++)
        {
            nv12image[image_y2 * pitch + x] = 0;
            nv12image[image_y2 * pitch + x + 1] = 0;
            nv12image[image_y2 * pitch + x + 2] = 0;
        }*/
    }
}

#define COLOR_COMPONENT_BIT_SIZE 10
#define COLOR_COMPONENT_MASK     0x3FF
// From OpenCV https://github.com/opencv/opencv/blob/master/modules/cudacodec/src/cuda/nv12_to_rgb.cu

extern "C"
__constant__ float constHueColorSpaceMat[9] = {1.1644f, 0.0f, 1.596f,
                                               1.1644f, -0.3918f, -0.813f,
                                               1.1644f, 2.0172f, 0.0f};

extern "C"
__device__ static void YUV2RGB(const unsigned int* yuvi, float* red, float* green, float* blue)
{
    float luma, chromaCb, chromaCr;

    // Prepare for hue adjustment
    luma     = (float)yuvi[0];
    chromaCb = (float)((int)yuvi[1] - 512.0f);
    chromaCr = (float)((int)yuvi[2] - 512.0f);

    // Convert YUV To RGB with hue adjustment
    red[0]   = (luma     * 1.1644f) +
             (chromaCb * 0.0f) +
             (chromaCr * 1.596f);

    green[0] = (luma     * 1.1644f) +
             (chromaCb * -0.3918f) +
             (chromaCr * -0.813f);

    blue[0]  = (luma     * 1.1644f) +
             (chromaCb * 2.0172f) +
             (chromaCr * 0.0f);
}

extern "C"
__device__ static unsigned int RGBA_pack_10bit(float red, float green, float blue, unsigned int alpha)
{
    unsigned int ARGBpixel = 0;

    // Clamp final 10 bit results
    red   = ::fmin(::fmax(red,   0.0f), 1023.f);
    green = ::fmin(::fmax(green, 0.0f), 1023.f);
    blue  = ::fmin(::fmax(blue,  0.0f), 1023.f);

    // Convert to 8 bit unsigned integers per color component
    ARGBpixel = (((unsigned int)blue  >> 2) |
      (((unsigned int)green >> 2) << 8)  |
      (((unsigned int)red   >> 2) << 16) |
      (unsigned int)alpha);

    return ARGBpixel;
}

extern "C"
__global__ void NV12_to_RGB(const unsigned char* srcImage, const unsigned int nSourcePitch,
                            unsigned int* dstImage, const unsigned int nDestPitch,
                            unsigned int width, unsigned int height)
{
    //const unsigned int nSourcePitch = 4096;

    // Pad borders with duplicate pixels, and we multiply by 2 because we process 2 pixels per thread
    const int x = blockIdx.x * (blockDim.x << 1) + (threadIdx.x << 1);
    const int y = blockIdx.y *  blockDim.y       +  threadIdx.y;

    if (x >= width || y >= height)
        return;

    // Read 2 Luma components at a time, so we don't waste processing since CbCr are decimated this way.
    // if we move to texture we could read 4 luminance values

    unsigned int yuv101010Pel[2];

    yuv101010Pel[0] = (srcImage[y * nSourcePitch + x    ]) << 2;
    yuv101010Pel[1] = (srcImage[y * nSourcePitch + x + 1]) << 2;

    const unsigned int chromaOffset = nSourcePitch * height;

    const int y_chroma = y >> 1;

    if (y & 1)  // odd scanline ?
    {
        unsigned int chromaCb = srcImage[chromaOffset + y_chroma * nSourcePitch + x    ];
        unsigned int chromaCr = srcImage[chromaOffset + y_chroma * nSourcePitch + x + 1];

        if (y_chroma < ((height >> 1) - 1)) // interpolate chroma vertically
        {
            chromaCb = (chromaCb + srcImage[chromaOffset + (y_chroma + 1) * nSourcePitch + x    ] + 1) >> 1;
            chromaCr = (chromaCr + srcImage[chromaOffset + (y_chroma + 1) * nSourcePitch + x + 1] + 1) >> 1;
        }

        yuv101010Pel[0] |= (chromaCb << ( COLOR_COMPONENT_BIT_SIZE       + 2));
        yuv101010Pel[0] |= (chromaCr << ((COLOR_COMPONENT_BIT_SIZE << 1) + 2));

        yuv101010Pel[1] |= (chromaCb << ( COLOR_COMPONENT_BIT_SIZE       + 2));
        yuv101010Pel[1] |= (chromaCr << ((COLOR_COMPONENT_BIT_SIZE << 1) + 2));
    }
    else
    {
        yuv101010Pel[0] |= ((unsigned int)srcImage[chromaOffset + y_chroma * nSourcePitch + x    ] << ( COLOR_COMPONENT_BIT_SIZE       + 2));
        yuv101010Pel[0] |= ((unsigned int)srcImage[chromaOffset + y_chroma * nSourcePitch + x + 1] << ((COLOR_COMPONENT_BIT_SIZE << 1) + 2));

        yuv101010Pel[1] |= ((unsigned int)srcImage[chromaOffset + y_chroma * nSourcePitch + x    ] << ( COLOR_COMPONENT_BIT_SIZE       + 2));
        yuv101010Pel[1] |= ((unsigned int)srcImage[chromaOffset + y_chroma * nSourcePitch + x + 1] << ((COLOR_COMPONENT_BIT_SIZE << 1) + 2));
    }

    // this steps performs the color conversion
    unsigned int yuvi[6];
    float red[2], green[2], blue[2];

    yuvi[0] =  (yuv101010Pel[0] &   COLOR_COMPONENT_MASK    );
    yuvi[1] = ((yuv101010Pel[0] >>  COLOR_COMPONENT_BIT_SIZE)       & COLOR_COMPONENT_MASK);
    yuvi[2] = ((yuv101010Pel[0] >> (COLOR_COMPONENT_BIT_SIZE << 1)) & COLOR_COMPONENT_MASK);

    yuvi[3] =  (yuv101010Pel[1] &   COLOR_COMPONENT_MASK    );
    yuvi[4] = ((yuv101010Pel[1] >>  COLOR_COMPONENT_BIT_SIZE)       & COLOR_COMPONENT_MASK);
    yuvi[5] = ((yuv101010Pel[1] >> (COLOR_COMPONENT_BIT_SIZE << 1)) & COLOR_COMPONENT_MASK);

    // YUV to RGB Transformation conversion
    YUV2RGB(&yuvi[0], &red[0], &green[0], &blue[0]);
    YUV2RGB(&yuvi[3], &red[1], &green[1], &blue[1]);

    // Clamp the results to RGBA

    const unsigned int dstImagePitch = nDestPitch >> 2;

    dstImage[y * dstImagePitch + x     ] = RGBA_pack_10bit(red[0], green[0], blue[0], ((unsigned int)0xff << 24));
    dstImage[y * dstImagePitch + x + 1 ] = RGBA_pack_10bit(red[1], green[1], blue[1], ((unsigned int)0xff << 24));
}