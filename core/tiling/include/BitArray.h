//
// Created by sophi on 4/15/2018.
//

#ifndef LIGHTDB_BITARRAY_H
#define LIGHTDB_BITARRAY_H

#include <vector>
#include <string>

namespace lightdb {
    namespace utility {

        class BitArray : public std::vector<bool> {

        public:

            BitArray(const size_t size) : vector(size) {}

            /**
             * Sets the byte at "location" to "value" in this BitArray
             * @param location Index into the bit array (unit of measurement being bytes)
             * @param data The bit array being modified
             * @param value The byte to store
             */
            void SetByte(const size_t location, unsigned char value);

            /**
             * Gets the byte at "location" in this BitArray
             * @param location Index into the bit array (unit of measurement being bytes)
             * @param data The bit array being accessed
             * @return The byte starting at location
             */
            unsigned char GetByte(const size_t location) const;

            /**
             * Replaces the bits starting at "start" and ending at "end" with "replacement" in
             * the this BitArray
             * @param start The bits before, but not including, start will be preserved
             * @param end The bits after, and including, end will be preserved
             * @param replacement The bits to be inserted between start and end - 1, inclusive
             */
            void Replace(const size_t start, const size_t end, const vector<bool> &replacement);

            /**
             * Inserts "value" at "location" in this BitArray, padding the front with bits until it is value_size
             * @param location The index that will follow insertion. All values at location and after
             * will now appear after the inserted bits
             * @param  value The integer value whose bits will be inserted into data
             * @param value_size The number of bits the value should take up
             */
            void Insert(const size_t location, size_t value, const size_t value_size);

            /**
             * Pads data with bits until it ends at a byte offset. Then, removes
             * all zero bytes from the end of the array
             * @param data The data to be padded
             */
            void ByteAlign();

            /**
             * Pads data with bits until it ends at a byte offset
             * @param data The data to be padded
             */
            void ByteAlignWithoutRemoval();


        };

        /**
         * Checks that index is within [start end]. Throws an index out of bounds exception
         * if not
         * @param index The index to be checked
         * @param start The first valid value of the index, inclusive
         * @param end, The last valid value of the index,  inclusive
         */
        void CheckBounds(const size_t index, const size_t start, const size_t end);
    }
}

#endif //LIGHTDB_BITARRAY_H
