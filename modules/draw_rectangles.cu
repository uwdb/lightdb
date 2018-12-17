#include "../core/utility/include/Rectangle.h"

extern "C"
__global__
void draw_rectangles(
        unsigned char* nv12output,
        const unsigned int height,
        const unsigned int width,
        const unsigned int pitch,
        const lightdb::Rectangle *rectangles,
        const unsigned int rectangle_count,
        const unsigned int line_width) {
    const int im_x = blockDim.x * blockIdx.x + threadIdx.x;
    const int im_y = blockDim.y * blockIdx.y + threadIdx.y;
    const unsigned int rectangle_id = blockDim.z * blockIdx.z + threadIdx.z;

    if(rectangle_id < rectangle_count && im_x < width && im_y < height) {
        const lightdb::Rectangle &b = rectangles[rectangle_id];
        const unsigned int output_luma_offset = im_x + im_y * pitch;
        const unsigned int output_luma_size = height * pitch;
        const unsigned int output_chroma_offset = output_luma_size + im_x + (im_y / 2) * pitch;

        const bool on_left_border =   im_x >= b.x &&
                                      im_x < b.x + line_width &&
                                      im_y >= b.y  &&
                                      im_y <= b.y + b.height;
        const bool on_right_border =  im_x > b.x + b.width - line_width &&
                                      im_x <= b.x + b.width &&
                                      im_y >= b.y &&
                                      im_y <= b.y + b.height;

        const bool on_top_border =    im_y >= b.y &&
                                      im_y < b.y + line_width &&
                                      im_x >= b.x + line_width &&
                                      im_x <= b.x + b.width - line_width;
        const bool on_bottom_border = im_y > b.y + b.height - line_width &&
                                      im_y <= b.y + b.height &&
                                      im_x >= b.x + line_width &&
                                      im_x <= b.x + b.width - line_width;

        if (b.width > 0 && (on_top_border || on_bottom_border || on_left_border || on_right_border)) {
            nv12output[output_luma_offset] = 76;
            nv12output[output_chroma_offset] = 64;
        }
    }
}
