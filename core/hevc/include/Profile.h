#ifndef LIGHTDB_PROFILE_H
#define LIGHTDB_PROFILE_H

namespace lightdb {

        /**
         * Returns the number of bits representing the profile size in a byte stream
         * based on the param below
         * @param max_num_sub_layers_minus1
         * @return The number of bits
         */
        unsigned long GetSizeInBits(const unsigned long max_num_sub_layers_minus1);

        static constexpr unsigned int kBaseSizeInBits = 96;

}

#endif //LIGHTDB_PROFILE_H
