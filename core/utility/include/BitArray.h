#ifndef LIGHTDB_BITARRAY_H
#define LIGHTDB_BITARRAY_H

#include <vector>
#include <iostream>
#include <climits>

namespace lightdb::utility {

    class BitArray : public std::vector<bool> {

    public:

        explicit BitArray(const size_t size) : vector(size) {}

        /**
         * Sets the byte at "location" to "value" in this BitArray
         * @param location Index into the bit array (unit of measurement being bytes)
         * @param data The bit array being modified
         * @param value The byte to store
         */
        inline void SetByte(const size_t location, unsigned char value) {
            CheckBounds(location, 0, this->size() / CHAR_BIT - 1);
            auto byte_offset = location * CHAR_BIT;
            for (auto i = byte_offset; i < byte_offset + 8; i++) {
                // Again, want to start from the left of the byte and store these as the earlier
                // bits into the bit vector
                (*this)[i] = value & 128;
                value <<= 1;
            }
        }

        /**
         * Gets the byte at "location" in this BitArray
         * @param location Index into the bit array (unit of measurement being bytes)
         * @param data The bit array being accessed
         * @return The byte starting at location
         */
        inline unsigned char GetByte(const size_t location) const {
            CheckBounds(location, 0, this->size() / CHAR_BIT - 1);
            auto byte_offset = location * CHAR_BIT;
            unsigned char value = 0;
            for (auto i = byte_offset; i < byte_offset + 8; i++) {
                // Again, want to start from the left of the byte and store these as the earlier
                // bits into the result
                value = (value << 1u) | (*this)[i];
            }
            return value;
        }

        /**
         * Replaces the bits starting at "start" and ending at "end" with "replacement" in
         * the this BitArray
         * @param start The bits before, but not including, start will be preserved
         * @param end The bits after, and including, end will be preserved
         * @param replacement The bits to be inserted between start and end - 1, inclusive
         */
        void Replace(size_t start, size_t end, const vector<bool> &replacement);

        /**
         * Inserts "value" at "location" in this BitArray, padding the front with bits until it is value_size
         * @param location The index that will follow insertion. All values at location and after
         * will now appear after the inserted bits
         * @param  value The integer value whose bits will be inserted into data
         * @param value_size The number of bits the value should take up
         */
        void Insert(size_t location, size_t value, size_t value_size);

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
        inline void ByteAlignWithoutRemoval() {
            // We need two mods in the case that it is already byte aligned. The first mod would return 0, so
            // CHAR_BIT - 0 = CHAR_BIT and we would add another full byte. The second mod results in 0, meaning
            //  we add no bytes
            auto extend = (CHAR_BIT - this->size() % CHAR_BIT) % CHAR_BIT;
            resize(size() + extend, false);
        }

    private:
        /**
         * Checks that index is within [start end]. Throws an index out of bounds exception
         * if not
         * @param index The index to be checked
         * @param start The first valid value of the index, inclusive
         * @param end, The last valid value of the index,  inclusive
         */
        inline static void CheckBounds(const size_t index, const size_t start, const size_t end) {
            if (index < start || index > end) {
                throw std::out_of_range("Index passed is out of range");
            }
        }
    };
}; //namespace lightdb::utility

#endif //LIGHTDB_BITARRAY_H
