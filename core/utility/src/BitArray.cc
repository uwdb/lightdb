#include "BitArray.h"

namespace lightdb::utility {

    void BitArray::Insert(const size_t location, size_t value, const size_t value_size) {
        CheckBounds(location, 0, size());
        // The new size of the data is the old size plus the number of bits the value
        // should occupy
        auto new_size = size() + value_size;
        reserve(new_size);

        // Fill the place where the value will be with zeroes (effectively zero padding the front)
        insert(begin() + location, value_size, false);

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
        CheckBounds(start, 0, size());
        CheckBounds(end, 0, size());
        if (start > end) {
            throw std::out_of_range("Start is greater than end");
        }
        // The new size is the size of the old array, minus the chunk to be replaced, plus
        // the size of the replacement
        auto new_size = (size() - (end - start)) + replacement.size();
        BitArray replaced(new_size);

        // Insert the chunk before start
        move(begin(), begin() + start, replaced.begin());
        // Insert the entirety of the replacement
        move(replacement.begin(), replacement.end(), replaced.begin() + start);
        // Insert the chunk starting at end
        move(begin() + end, this->end(), replaced.begin() + start + replacement.size());

        *this = replaced;
    }

    void BitArray::ByteAlign() {
        // We need two mods in the case that it is already byte aligned. The first mod would return 0, so
        // CHAR_BIT - 0 = CHAR_BIT and we would add another full byte. The second mod results in 0, meaning
        //  we add no bytes
        auto extend = (CHAR_BIT - size() % CHAR_BIT) % CHAR_BIT;
        resize(size() + extend, false);
        // Remove the trailing zero bytes
        while (static_cast<int>(GetByte(size() / CHAR_BIT - 1)) == 0) {
            for (int i = 0u; i < CHAR_BIT; i++) {
                pop_back();
            }
        }
    }

}; //namespace lightdb::utility

