//
// Created by sophi on 5/12/2018.
//

#include <vector>
#include <list>
#include <cmath>
#include <iostream>

#include "Golombs.h"
#include "BitArray.h"
#include "BitStream.h"

using lightdb::utility::BitArray;
using lightdb::utility::BitStream;

namespace lightdb {

    /**
     * Returns the number of bits necessary to encode this golomb, storing the number of zeroes
     * needed at the front of the golomb at the end of the list
     * @param val The integer to be encoded as a golomb
     * @param zero_sizes The list that will hold the number of zeroes
     * @return The number of bits necessary to encode the golomb
     */
    size_t GetGolombEncodeSize(const unsigned long val, std::list<size_t> *zero_sizes);

    /**
    *
    * @param stream The stream to process, the iterator of which points to the first 0
    * of the golomb
    * @return The number of bits to interpret next in the stream as part of the golomb
    */
    size_t GetGolombDecodeSize(BitStream &stream);

    /**
     * Encodes "golomb" as a golomb, storing the bits in "data" starting at "index"
     * @param golomb The integer to encode as a golomb
     * @param data The BitArray that will hold the encoded bits
     * @param zero_sizes The number of zeroes this golomb will have preceding it, with the value stored at the front
     * of the list
     * @param index The index to start inserting the bits at in "data"
     * @return The number of bits used to encode this golomb
     */
    size_t EncodeGolomb(unsigned long golomb, BitArray &data, std::list<size_t> *zero_sizes, size_t index);

    BitArray EncodeGolombs(const std::vector<unsigned long> &golombs) {
        size_t total_size = 0u;
        std::list<size_t> zero_sizes;

        // Record the total size of the bit array
        for (auto val : golombs) {
            total_size += GetGolombEncodeSize(val, &zero_sizes);
        }

        BitArray data(total_size);
        size_t bit_index = 0;

        for (auto val : golombs) {
            bit_index = EncodeGolomb(val, data, &zero_sizes, bit_index);
        }
        return data;
    }

    size_t GetGolombEncodeSize(const unsigned long val, std::list<size_t> *zero_sizes) {

        // Note that val will always be at least 0, so val + 1 is always at least 1, meaning
        // log2 is never undefined
        auto val_size = static_cast<size_t>(log2(val + 1)) + 1;

        // The number of zero bits at the front is val_size - 1
        zero_sizes->push_back(val_size - 1);

        // Add the number of 0s necessary, which is val_size - 1, to make a total of
        // val_size + val_size - 1
        return val_size * 2 - 1;
    }

    size_t EncodeGolomb(unsigned long golomb, BitArray &data, std::list<size_t> *zero_sizes, size_t index) {
        golomb += 1;
        auto zero_num = zero_sizes->front();
        zero_sizes->pop_front();

        // First, add the zeroes at the front
        for (auto i = 0u; i < zero_num; i++) {
            data[index++] = false;
        }

        // The size of the value is actually zero_num + 1. But we want the least significant
        // bit to be stored at bit_index + val_size - 1, so that's just bit_index + zero_num
        auto val_start = index + zero_num;
        // Loop through the bits in the integer + 1 and add them to the BitArray
        while (golomb != 0) {
            // Again, want the high order bits earlier in the stream, so we need
            // to store the lowest order bits last
            data[val_start--] = static_cast<bool>(golomb & 1);
            golomb >>= 1;
            index++;
        }
        return index;
    }

    unsigned long DecodeGolomb(BitStream &stream) {
        auto size = GetGolombDecodeSize(stream);
        // Note that getting the size means that we skip the first "1" of the
        // exponential golomb due to the how the while loop is set up in that
        // method. Thus, when we try to extract the remaining bits of the golomb,
        // we actually need to get the next bits - 1 (which is exactly size, since
        // size is the number of bits in the golomb - 1). To add the 1 we skipped
        // at the beginning we or the result with 1 << size, and finally subtract 1
        // (due to the rules for golomb encoding).

        // I.e., the golomb is 00101. Index is left at the 0 following the 1 after a call
        // to get size. We then read in the next two zeroes (reading "size" number of bits),
        // leaving index at the bit after the golomb. We shift 1 over by two to get "100", and
        // we or it with the result we got (01), to get 101, and subtract 1.
        return (1u << size | stream.NextBits(size)) - 1;
    }

    size_t GetGolombDecodeSize(BitStream &stream) {
        // If the golomb is 00101, then we read the 0, update size to 1, read the next 0, update
        // size to 2, and receive a 1 next so we stop, leaving index at the 0 after the 1.
        auto size = 0u;
        while (stream.NextBits() == 0) {
            size += 1;
        }
        return size;
    }
}
