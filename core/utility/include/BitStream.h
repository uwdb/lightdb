#ifndef LIGHTDB_BITSTREAM_H
#define LIGHTDB_BITSTREAM_H

#include "Golombs.h"
#include <unordered_map>
#include <vector>
#include <cassert>


namespace lightdb {
    namespace utility {

        class BitStream {
        public:
            /**
             * Creates a bit stream with the byte array data as the underlying data,
             * starting at a byte offset of "start"
             * @param data The byte array
             * @param start The byte offset that marks the first position in this BitStream
             */
            template<typename Input>
            BitStream(Input start, Input current) : index_(current), first_index_(start) {
            }


            /**
             *
             * @return The next exponential golomb in the bit stream
             */
            inline unsigned long GetExponentialGolomb() {
                return DecodeGolomb(*this);
            }

            /**
             * Stores the value of "name" as the current position in the bit stream
             * @param name The name that the position will be associated with
             */
            inline BitStream &MarkPosition(const std::string &name) {
                values_[name] = static_cast<unsigned long>(std::distance(first_index_, index_));
                return *this;
            }

            /**
             * Skips the next exponential golomb
             */
            inline BitStream &SkipExponentialGolomb() {
                GetExponentialGolomb();
                return *this;
            }

            /**
             * Skips the next num exponential golombs
             * @param num The number to skip, 1 if nothing is specified
             */
            inline BitStream &SkipExponentialGolombs(const unsigned long num) {
                for (auto i = 0u; i < num; i++) {
                    SkipExponentialGolomb();
                }
                return *this;
            }

            /**
             * Skips the next num exponential golombs if the bit associated with
             * the key is 1
             * @param num The number to skip, 1 if nothing is specified
             * @param key The name that the bit to check is associated with
             */
            inline BitStream &SkipExponentialGolombs(const std::string &key, const size_t num = 1) {
                if (values_[key]) {
                    SkipExponentialGolombs(num);
                }
                return *this;
            }

            /**
             * Skips the next num bits in the stream
             * @param num The number of bits to skip
             */
            inline BitStream &SkipBits(const size_t num) {
                index_ += num;
                return *this;
            }

            /**
             * Skips the next num bits in the stream if skip is true
             * @param num The number of bits to skip
             * @param skip Determines whether or not to skip them
             */
            inline BitStream &SkipBits(const size_t num, const bool skip) {
                if (skip) {
                    SkipBits(num);
                }
                return *this;
            }

            /**
             * Skips the next bit in the stream, checking that it is a 1
             */
            inline BitStream &SkipTrue() {
                auto bit = NextBits();
                assert (bit);
                return *this;
            }

            /**
             * Skips the next bit in the stream, checking that it is a 0
             */
            inline BitStream &SkipFalse() {
                auto bit = NextBits();
                assert (!bit);
                return *this;
            }

            /**
             * Stores the value of "name" as the next bit(s) in the stream
             * @param name The name that the bits will be associated with
             * @param num The number of bits to store
             */
            inline BitStream &CollectValue(const std::string &name, const size_t num = 1) {
                auto bits = NextBits(num);
                values_[name] = bits;
                return *this;
            }

            /**
             * Stores the value of "name" as the next bit in the stream
             * @param name The name that the bit will be associated with
             * @param expected The expected value of the bit
             */
            inline BitStream &CollectValue(const std::string &name, const size_t num, bool expected) {
                auto bit = NextBits(num);
                assert (bit == expected);
                values_[name] = bit;
                return *this;
            }

            /**
             * Stroes the value of "name" as the next golomb in the stream
             * @param name The name the golomb will be associated with
             */
            inline BitStream &CollectGolomb(const std::string &name) {
                auto golomb = GetExponentialGolomb();
                values_[name] = golomb;
                return *this;
            }

            /**
             * Aligns the current index of the BitStream to the next byte offset
             * (so it moves the index forward, if necessary)
             * @param expected The expected number of bits skipped in the bit stream to
             * align it. Only checked if some value is passed, otherwise set to a default of
             * -1 and not checked
             */
            inline BitStream &ByteAlign() {
                ByteAlign(false, 0);
                return *this;
            }

            /**
             * Aligns the current index of the BitStream to the next byte offset
             * (so it moves the index forward, if necessary)
             * @param expected The expected value of the bits that were skipped to
             * align the stream
             */
            inline BitStream &ByteAlign(size_t expected) {
                ByteAlign(true, expected);
                return *this;
            }

            /**
             * Skips the entry point offsets in the bit stream if skip is true
             * @param skip Determines whether or not to skip the offsets
             */
            inline BitStream &SkipEntryPointOffsets(bool skip) {
                if (skip) {
                    auto num_entry_point_offsets = GetExponentialGolomb();
                    if (num_entry_point_offsets) {
                        auto offset_len_minus1 = GetExponentialGolomb();
                        SkipBits((offset_len_minus1 + 1) * num_entry_point_offsets);
                    }
                }
                return *this;
            }

            /**
             * Returns the bit(s) associated with name
             * @param name The name the bits are associated with. Name must have been passed to a call to
             * CollectBit earlier
             * @return The bits
             */
            inline unsigned long GetValue(const std::string &name) const {
                return values_.at(name);
            }

            /**
             * Returns the next num of bits, leaving the iterator pointing at the next
             * unprocessed bit in the stream
             * @param num The number of bits
             * @return Num bits
             */
            inline unsigned long NextBits(size_t num = 1) {
                    unsigned long bits = 0u;
                    for (auto i = 0u; i < num; i++) {
                            bits = (bits << 1) | *index_++;
                    }
                    return bits;
            }

            BitStream(const BitStream& other) = default;
            BitStream(BitStream&& other) = default;
            //BitStream& operator= (const BitStream& other) = default;
            //BitStream& operator= (BitStream&& other) = default;
            ~BitStream() = default;

        private:

            inline void ByteAlign(const bool check, const size_t expected) {
                // The extra % 8 is to handle the case where it is already byte aligned -
                // in that case index % 8 will be 0, 8 - 0 = 8, and we will go to the next
                // byte. Instead, we want to stay at the current byte, so we add an extra
                // % 8 to make it 0
                auto value = NextBits(static_cast<size_t>((8 - std::distance(first_index_, index_) % 8) % 8));
                SkipBits(value);

                assert (!check || value == expected);
            }

            std::unordered_map<std::string, unsigned long> values_;

            // Represents the bit offset into the bit stream
            std::vector<bool>::const_iterator index_;
            // Represents the starting index of the bit stream
            const std::vector<bool>::const_iterator first_index_;
        };
    }
}

#endif //LIGHTDB_BITSTREAM_H