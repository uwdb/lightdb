#ifndef LIGHTDB_GOLOMBS_H
#define LIGHTDB_GOLOMBS_H

#include "BitArray.h"

namespace lightdb {
    namespace utility { class BitStream; };

    /**
     * Encodes the values in golombs as golombs, in the order provided
     * @param ints The integers to be encoded as ints
     * @returns A BitArray holding the encoded contents
     */
    utility::BitArray EncodeGolombs(const std::vector<unsigned long> &golombs);

    /**
     * Decodes the next golomb in the stream, returning the value as a long
     * @param stream The stream that contains the bits of the golomb, with its iterator
     * set to the first bit
     * @return The golomb
     */
    unsigned long DecodeGolomb(utility::BitStream &stream);
}

#endif //LIGHTDB_GOLOMBS_H
