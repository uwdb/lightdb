#include "BitArray.h"
#include <gtest/gtest.h>
#include <climits>

using lightdb::BitArray;

static const unsigned long kArraySize = 64u;

class BitArrayTestFixture : public testing::Test {
public:

    BitArrayTestFixture()
            : array(kArraySize)
    { }

    BitArray array;
};

TEST_F(BitArrayTestFixture, testConstructor) {
    ASSERT_TRUE(array.size() == kArraySize);
}

TEST_F(BitArrayTestFixture, testSetByteGetByte) {

    for (auto i = 0u; i < kArraySize / 8; i++) {
        array.SetByte(i, static_cast<unsigned char>(i));
    }

    for (auto i = 0u; i < kArraySize / 8; i++) {
        unsigned char byte = 0;
        for (auto j = i * CHAR_BIT; j < i * CHAR_BIT + CHAR_BIT; j++) {
            byte = (byte << 1u) | array[j];
        }
        ASSERT_EQ(byte, i);
    }

    for (auto i = 0u; i < kArraySize / 8; i++) {
        auto byte = array.GetByte(i);
        ASSERT_EQ(byte, i);
    }
}

TEST_F(BitArrayTestFixture, testSetByteException) {
    ASSERT_THROW(array.SetByte(-1, static_cast<unsigned char>(0)), std::out_of_range);
    ASSERT_THROW(array.SetByte(57, static_cast<unsigned char>(0)), std::out_of_range);
}

TEST_F(BitArrayTestFixture, testInsertException) {
    ASSERT_THROW(array.Insert(-1, 0, 3), std::out_of_range);
    ASSERT_THROW(array.Insert(kArraySize + 1, 0, 3), std::out_of_range);
}

TEST_F(BitArrayTestFixture, testReplaceException) {
    std::vector<bool> replacement(10);
    ASSERT_THROW(array.Replace(-3, -1, replacement), std::out_of_range);
    ASSERT_THROW(array.Replace(kArraySize + 1, kArraySize + 1, replacement), std::out_of_range);
    ASSERT_THROW(array.Replace(5, kArraySize + 1, replacement), std::out_of_range);
    ASSERT_THROW(array.Replace(5, 4, replacement), std::out_of_range);
}

TEST_F(BitArrayTestFixture, testInsert) {
    for (auto i = 0u; i < kArraySize; i++) {
        array[i] = true;
    }

    // Simple insertion
    auto val_size = 2u;
    auto array_size = kArraySize + val_size;
    array.Insert(50, 1u, val_size);
    ASSERT_FALSE(array[50]);
    ASSERT_TRUE(array[51]);
    ASSERT_EQ(array.size(), array_size);

    // More complicated
    val_size = 11;
    array_size += val_size;
    array.Insert(30, 292u, val_size);
    ASSERT_FALSE(array[30]);
    ASSERT_FALSE(array[31]);
    auto num = 0u;
    for (auto i = 32u; i < 41u; i++) {
        num = (num << 1u) | array[i];
    }
    ASSERT_EQ(num, 292u);
    ASSERT_EQ(array.size(), array_size);

    // Make sure that it doesn't pad when it doesn't need to
    val_size = 10;
    array_size += val_size;
    array.Insert(20, 542u, val_size);
    num = 0;
    for (auto i = 20u; i < 30u; i++) {
        num = (num << 1u) | array[i];
    }
    ASSERT_EQ(num, 542u);
    ASSERT_EQ(array.size(), array_size);

    // Make sure inserting at the end is valid as well
    val_size = 3;
    array.Insert(array_size, 2u, val_size);
    ASSERT_FALSE(array[array_size]);
    ASSERT_TRUE(array[array_size + 1]);
    ASSERT_FALSE(array[array_size + 2]);
    array_size += val_size;
    ASSERT_EQ(array.size(), array_size);
}

TEST_F(BitArrayTestFixture, testReplace) {
    auto replacement_size = 10u;
    std::vector<bool> replacement(replacement_size);

    for (auto i = 0u; i < kArraySize; i++) {
        array[i] = true;
    }
    for (auto i = 0u; i < replacement_size; i++) {
        replacement[i] = false;
    }

    // Make sure we can correctly insert a smaller replacement
    array.Replace(10, 40, replacement);
    for (auto i = 0u; i < 10u; i++) {
        ASSERT_TRUE(array[i]);
    }
    for (auto i = 10u; i < 10u + replacement_size; i++) {
        ASSERT_FALSE(array[i]);
    }
    for (auto i = 10u + replacement_size; i < array.size(); i++) {
        ASSERT_TRUE(array[i]);
    }
    ASSERT_EQ(array.size(), kArraySize - 40u + 10u + replacement_size);

    // Make sure we can correctly insert a larger replacement
    array = BitArray(kArraySize);
    for (auto i = 0u; i < kArraySize; i++) {
        array[i] = true;
    }
    array.Replace(10, 15, replacement);
    for (auto i = 0u; i < 10; i++) {
        ASSERT_TRUE(array[i]);
    }
    for (auto i = 10u; i < 10 + replacement_size; i++) {
        ASSERT_FALSE(array[i]);
    }
    for (auto i = 10u + replacement_size; i < array.size(); i++) {
        ASSERT_TRUE(array[i]);
    }
    ASSERT_EQ(array.size(), kArraySize - 15 + 10 + replacement_size);

    // Make sure we can correctly insert a larger replacement
    array = BitArray(kArraySize);
    for (auto i = 0u; i < kArraySize; i++) {
        array[i] = true;
    }
    array.Replace(10, 10 + replacement_size, replacement);
    for (auto i = 0u; i < 10; i++) {
        ASSERT_TRUE(array[i]);
    }
    for (auto i = 10u; i < 10 + replacement_size; i++) {
        ASSERT_FALSE(array[i]);
    }
    for (auto i = 10u + replacement_size; i < array.size(); i++) {
        ASSERT_TRUE(array[i]);
    }
    ASSERT_EQ(array.size(), kArraySize);
}

TEST_F(BitArrayTestFixture, testByteAlignWithoutRemoval) {
    // Add two
    array.push_back(true);
    array.push_back(true);
    array.ByteAlignWithoutRemoval();

    // Should be one byte bigger now
    ASSERT_EQ(array.size(), kArraySize + CHAR_BIT);

    // Should have been padded with zeroes
    for (auto i = kArraySize + 2u; i < array.size(); i++) {
        ASSERT_FALSE(array[i]);
    }

    // Already byte aligned, shouldn't change size
    array.ByteAlignWithoutRemoval();
    ASSERT_EQ(array.size(), kArraySize + CHAR_BIT);

    // Now, add zeroes and check that it aligns but does not remove the
    // 0 byte
    for (auto i = 0; i < CHAR_BIT - 1; i++) {
        array.push_back(false);
    }

    array.ByteAlignWithoutRemoval();
    ASSERT_EQ(array.size(), kArraySize + 2 * CHAR_BIT);
}

TEST_F(BitArrayTestFixture, testByteAlignWithRemoval) {
    // Add two
    array.push_back(true);
    array.push_back(true);
    array.ByteAlign();

    // Should be one byte bigger now
    ASSERT_EQ(array.size(), kArraySize + CHAR_BIT);

    // Should have been padded with zeroes
    for (auto i = kArraySize + 2u; i < array.size(); i++) {
        ASSERT_FALSE(array[i]);
    }

    // Already byte aligned, shouldn't change size
    array.ByteAlign();
    ASSERT_EQ(array.size(), kArraySize + CHAR_BIT);

    // Now, check add some values and check that the trailing zeroes are removed
    for (auto i = 0; i < 3; i++) {
        for (auto j = 0; j < CHAR_BIT; j++) {
            array.push_back(false);
        }
    }

    for (auto i = 0; i < CHAR_BIT - 1; i++) {
        array.push_back(false);
    }

    // Should have added three bytes in total, should now remove last four
    // and preserve original size
    array.ByteAlign();
    ASSERT_EQ(array.size(), kArraySize + CHAR_BIT);
}

