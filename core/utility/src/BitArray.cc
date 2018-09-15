#include "BitArray.h"
#include <iostream>
#include <climits>

using std::move;

namespace lightdb::utility {

    void CheckBounds(const size_t index, const size_t start, const size_t end) {
        if (index < start || index > end) {
            throw std::out_of_range("Index passed is out of range");
        }
    }


    void BitArray::SetByte(const size_t location, unsigned char value) {
        CheckBounds(location, 0, this->size() / CHAR_BIT - 1);
        auto byte_offset = location * CHAR_BIT;
        for (auto i = byte_offset; i < byte_offset + 8; i++) {
            // Again, want to start from the left of the byte and store these as the earlier
            // bits into the bit vector
            (*this)[i] = value & 128;
            value <<= 1;
        }
    }

    unsigned char BitArray::GetByte(const size_t location) const {
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

    void BitArray::Insert(const size_t location, size_t value, const size_t value_size) {
        CheckBounds(location, 0, this->size());
        // The new size of the data is the old size plus the number of bits the value
        // should occupy
        auto new_size = this->size() + value_size;
        this->reserve(new_size);

        // Fill the place where the value will be with zeroes (effectively zero padding the front)
        this->insert(this->begin() + location, value_size, false);

        // We want the least significant bit to be stored at location + value_size - 1,
        // so that's where we start
        auto val_end = location + value_size;
        auto index = val_end - 1;
        // To ensure that the right shift is unsigned
        auto u_value = static_cast<unsigned int>(value);

        // Insert the bits of value starting at val_end, and ending at the
        // index right after the zero padding
        while (u_value != 0) {
            (*this)[index--] = u_value & 1;
            u_value >>= 1;
        }
    }

    void BitArray::Replace(const size_t start, const size_t end, const vector<bool> &replacement) {
        CheckBounds(start, 0, this->size());
        CheckBounds(end, 0, this->size());
        if (start > end) {
            throw std::out_of_range("Start is greater than end");
        }
        // The new size is the size of the old array, minus the chunk to be replaced, plus
        // the size of the replacement
        auto new_size = (this->size() - (end - start)) + replacement.size();
        BitArray replaced(new_size);

        // Insert the chunk before start
        move(this->begin(), this->begin() + start, replaced.begin());
        // Insert the entirety of the replacement
        move(replacement.begin(), replacement.end(), replaced.begin() + start);
        // Insert the chunk starting at end
        move(this->begin() + end, this->end(), replaced.begin() + start + replacement.size());

        *this = replaced;
    }

    void BitArray::ByteAlign() {
        // We need two mods in the case that it is already byte aligned. The first mod would return 0, so
        // CHAR_BIT - 0 = CHAR_BIT and we would add another full byte. The second mod results in 0, meaning
        //  we add no bytes
        auto extend = (CHAR_BIT - this->size() % CHAR_BIT) % CHAR_BIT;
        this->resize(this->size() + extend, false);
        // Remove the trailing zero bytes
        while (static_cast<int>(this->GetByte(this->size() / CHAR_BIT - 1)) == 0) {
            for (int i = 0; i < CHAR_BIT; i++) {
                this->pop_back();
            }
        }
    }

    void BitArray::ByteAlignWithoutRemoval() {
        // We need two mods in the case that it is already byte aligned. The first mod would return 0, so
        // CHAR_BIT - 0 = CHAR_BIT and we would add another full byte. The second mod results in 0, meaning
        //  we add no bytes
        auto extend = (CHAR_BIT - this->size() % CHAR_BIT) % CHAR_BIT;
        this->resize(this->size() + extend, false);
    }
}; //namespace lightdb::utility

