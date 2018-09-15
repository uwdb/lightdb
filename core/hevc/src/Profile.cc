#include "Profile.h"
#include <cassert>

namespace lightdb {

    unsigned long GetSizeInBits(const unsigned long max_num_sub_layers_minus1) {
        auto sub_layer_size = 2 * max_num_sub_layers_minus1;
        auto reserved_zero_2bits_size = 0;
        if (max_num_sub_layers_minus1 > 0) {
            reserved_zero_2bits_size = 2 * 8;
        }
        auto size = kBaseSizeInBits + sub_layer_size + reserved_zero_2bits_size;
        assert (size % 8 == 0);
        return size;
    }
}

