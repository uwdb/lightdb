#ifndef LIGHTDB_BITSTREAM_H
#define LIGHTDB_BITSTREAM_H

#include <string>
#include <unordered_map>
#include <vector>
#include <cstddef>
#include <iterator>


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
            BitStream(Input start, Input current) : start_(start), index_(current), first_index_(start) {}


            /**
             *
             * @return The next exponential golomb in the bit stream
             */
            unsigned long GetExponentialGolomb();

            /**
             * Stores the value of "name" as the current position in the bit stream
             * @param name The name that the position will be associated with
             */
            BitStream &MarkPosition(const std::string &name);

            /**
             * Skips the next num exponential golombs
             * @param num The number to skip, 1 if nothing is specified
             */
            BitStream &SkipExponentialGolombs(const unsigned long num);

            /**
             * Skips the next exponential golomb
             */
            BitStream &SkipExponentialGolomb();

            /**
             * Skips the next num exponential golombs if the bit associated with
             * the key is 1
             * @param num The number to skip, 1 if nothing is specified
             * @param key The name that the bit to check is associated with
             */
            BitStream &SkipExponentialGolombs(const std::string &key, const size_t num = 1);

            /**
             * Skips the next num bits in the stream
             * @param num The number of bits to skip
             */
            BitStream &SkipBits(const size_t num);

            /**
             * Skips the next num bits in the stream if skip is true
             * @param num The number of bits to skip
             * @param skip Determines whether or not to skip them
             */
            BitStream &SkipBits(const size_t num, bool skip);

            /**
             * Skips the next bit in the stream, checking that it is a 1
             */
            BitStream &SkipTrue();

            /**
             * Skips the next bit in the stream, checking that it is a 0
             */
            BitStream &SkipFalse();

            /**
             * Stores the value of "name" as the next bit(s) in the stream
             * @param name The name that the bits will be associated with
             * @param expected The expected value of the bit
             * @param num The number of bits to store
             */
            BitStream &CollectValue(const std::string &name, const size_t num = 1);

            /**
             * Stroes the value of "name" as the next golomb in the stream
             * @param name The name the golomb will be associated with
             */
            BitStream &CollectGolomb(const std::string &name);

            /**
             * Aligns the current index of the BitStream to the next byte offset
             * (so it moves the index forward, if necessary)
             * @param expected The expected number of bits skipped in the bit stream to
             * align it. Only checked if some value is passed, otherwise set to a default of
             * -1 and not checked
             */
            BitStream &ByteAlign();

            /**
             * Aligns the current index of the BitStream to the next byte offset
             * (so it moves the index forward, if necessary)
             * @param expected The expected number of bits skipped in the bit stream to
             * align it.
             */
            BitStream &ByteAlign(const size_t expected);

            /**
             * Skips the entry point offsets in the bit stream if skip is true
             * @param skip Determines whether or not to skip the offsets
             */
            BitStream &SkipEntryPointOffsets(bool skip);

            /**
             * Returns the bit(s) associated with name
             * @param name The name the bits are associated with. Name must have been passed to a call to
             * CollectBit earlier
             * @return The bits
             */
            unsigned long GetValue(const std::string &name) const;

            /**
             * Returns the next num of bits, leaving the iterator pointing at the next
             * unprocessed bit in the stream
             * @param num The number of bits
             * @return Num bits
             */
            unsigned long NextBits(const size_t num = 1);

        private:


            void ByteAlign(const bool check, const size_t expected);

            std::unordered_map<std::string, unsigned long> values_;

            // Represents the bit offset into the bit stream
            std::vector<bool>::const_iterator index_;
            const std::vector<bool>::const_iterator start_;
            const std::vector<bool>::const_iterator first_index_;
        };
    }
}

#endif //LIGHTDB_BITSTREAM_H
