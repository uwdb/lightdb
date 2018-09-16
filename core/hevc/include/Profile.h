#ifndef LIGHTDB_PROFILE_H
#define LIGHTDB_PROFILE_H

#include <cassert>

namespace lightdb {

    static constexpr unsigned int kBaseSizeInBits = 96u;

    /**
     * Returns the number of bits representing the profile size in a byte stream
     * based on the param below
     * @param max_num_sub_layers_minus1
     * @return The number of bits
     */
    inline unsigned long GetSizeInBits(const unsigned long max_num_sub_layers_minus1) {
            auto sub_layer_size = 2 * max_num_sub_layers_minus1;
            auto reserved_zero_2bits_size = 0;
            if (max_num_sub_layers_minus1 > 0) {
                    reserved_zero_2bits_size = 2 * 8;
            }
            auto size = kBaseSizeInBits + sub_layer_size + reserved_zero_2bits_size;
            assert (size % 8 == 0);
            return size;
    }
}; //namespace lightdb

#endif //LIGHTDB_PROFILE_H
