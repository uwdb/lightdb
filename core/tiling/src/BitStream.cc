//
// Created by sophi on 4/13/2018.
//

#include <vector>
#include <iostream>
#include <map>
#include <cassert>
#include "BitStream.h"
#include "BitArray.h"
#include "Golombs.h"


namespace lightdb {
    namespace utility {

        BitStream &BitStream::SkipTrue() {
            auto bit = NextBits();
            assert (bit);
            return *this;
        }

        BitStream &BitStream::SkipFalse() {
            auto bit = NextBits();
            assert (!bit);
            return *this;
        }

        BitStream &BitStream::SkipBits(const size_t num) {
            index_ += num;
            return *this;
        }

        BitStream &BitStream::SkipBits(const size_t num, bool skip) {
            if (skip) {
                SkipBits(num);
            }
        }

        BitStream &BitStream::CollectValue(const std::string &name, const size_t num) {
            auto bits = NextBits(num);
            values_[name] = bits;
            return *this;
        }

        BitStream &BitStream::CollectValue(const std::string &name, const size_t num, bool expected) {
            auto bit = NextBits(num);
            assert (bit == expected);
            values_[name] = bit;
            return *this;
        }

        BitStream &BitStream::CollectGolomb(const std::string &name) {
            auto golomb = GetExponentialGolomb();
            values_[name] = golomb;
            return *this;
        }

        unsigned long BitStream::GetValue(const std::string &name) const {
            return values_.at(name);
        }

        unsigned long BitStream::NextBits(const size_t num) {
            unsigned long bits = 0;
            for (auto i = 0; i < num; i++) {
                bits = (bits << 1) | *index_++;
            }
            return bits;
        }

        BitStream &BitStream::SkipExponentialGolombs(const std::string &key, const size_t num) {
            if (values_[key]) {
                SkipExponentialGolombs(num);
            }
            return *this;
        }

        BitStream &BitStream::SkipExponentialGolombs(const unsigned long num) {
            for (auto i = 0; i < num; i++) {
                SkipExponentialGolomb();
            }
            return *this;
        }

        BitStream &BitStream::SkipExponentialGolomb() {
            GetExponentialGolomb();
            return *this;
        }

        BitStream &BitStream::MarkPosition(const std::string &name) {
            values_[name] = static_cast<unsigned long>(std::distance(first_index_, index_));
            return *this;
        }

        BitStream &BitStream::SkipEntryPointOffsets(bool skip) {
            if (skip) {
                auto num_entry_point_offsets = GetExponentialGolomb();
                if (num_entry_point_offsets) {
                    auto offset_len_minus1 = GetExponentialGolomb();
                    SkipBits((offset_len_minus1 + 1) * num_entry_point_offsets);
                }
            }
            return *this;
        }

        unsigned long BitStream::GetExponentialGolomb() {
            return DecodeGolomb(*this);
        }

        BitStream &BitStream::ByteAlign(const size_t expected) {
            ByteAlign(true, expected);
            return *this;
        }

        BitStream &BitStream::ByteAlign() {
            ByteAlign(false, 0);
            return *this;
        }

        void BitStream::ByteAlign(const bool check, const size_t expected) {
            // The extra % 8 is to handle the case where it is already byte aligned -
            // in that case index % 8 will be 0, 8 - 0 = 8, and we will go to the next
            // byte. Instead, we want to stay at the current byte, so we add an extra
            // % 8 to make it 0
            auto value = NextBits(static_cast<size_t>((8 - std::distance(first_index_, index_) % 8) % 8));
            SkipBits(value);
            if (check) {
                assert (value == expected);
            }
        }
    }
}
