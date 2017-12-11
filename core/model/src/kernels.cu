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
