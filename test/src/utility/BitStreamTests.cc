#include "BitStream.h"
#include <gtest/gtest.h>

using lightdb::BitStream;
static constexpr unsigned int kDataSize = 50u;

class BitStreamTestFixture : public testing::Test {
};

TEST_F(BitStreamTestFixture, testMarkPositionSkipBits) {
    std::vector<bool> data(kDataSize);
    BitStream stream(data.begin(), data.begin() + 5);

    stream.MarkPosition("start");
    stream.SkipBits(10);
    stream.MarkPosition("middle");
    stream.SkipBits(kDataSize - 5 - 10);
    stream.MarkPosition("end");

    ASSERT_EQ(stream.GetValue("start"), 5);
    ASSERT_EQ(stream.GetValue("middle"), 15);
    ASSERT_EQ(stream.GetValue("end"), kDataSize);
}

TEST_F(BitStreamTestFixture, testByteAlign) {
    std::vector<bool> data(kDataSize);
    for (auto i = 20u; i < 25u; i++) {
        data[i] = true;
    }
    BitStream stream(data.begin(), data.begin() + 5);

    stream.SkipBits(15);
    stream.ByteAlign(15);
    stream.SkipBits(11);
    stream.ByteAlign(0);
    stream.SkipBits(16);
    stream.ByteAlign(0);
}

TEST_F(BitStreamTestFixture, testSkipEntryPointOffsets) {
    std::vector<bool> data(kDataSize);

    // 3
    data[0] = false;
    data[1] = false;
    data[2] = true;
    data[3] = false;
    data[4] = false;
    // 2
    data[5] = false;
    data[6] = true;
    data[7] = true;
    // 9 bits to skip
    data[8] = false;
    data[9] = false;
    data[10] = false;
    data[11] = false;
    data[12] = false;
    data[13] = false;
    data[14] = false;
    data[15] = false;
    data[16] = false;
    // Mark bit
    data[17] = true;
    // 0
    data[18] = true;
    // shouldn't read this next value
    data[19] = false;
    data[20] = true;
    data[21] = true;
    // shouldn't process this either
    data[22] = false;

    BitStream stream(data.begin(), data.begin());

    stream.SkipEntryPointOffsets(true);
    stream.SkipTrue();
    stream.MarkPosition("first-mark");
    ASSERT_EQ(stream.GetValue("first-mark"), 18);
    stream.SkipEntryPointOffsets(true);
    stream.MarkPosition("second-mark");
    ASSERT_EQ(stream.GetValue("second-mark"), 19);
    stream.SkipEntryPointOffsets(false);
    stream.MarkPosition("third-mark");
    ASSERT_EQ(stream.GetValue("third-mark"), 19);
}

TEST_F(BitStreamTestFixture, testGetExponentialGolomb) {
    std::vector<bool> data(kDataSize);

    // 3
    data[0] = false;
    data[1] = false;
    data[2] = true;
    data[3] = false;
    data[4] = false;
    // 0
    data[5] = true;
    // 8
    data[6] = false;
    data[7] = false;
    data[8] = false;
    data[9] = true;
    data[10] = false;
    data[11] = false;
    data[12] = true;
    BitStream stream(data.begin(), data.begin());

    ASSERT_EQ(stream.GetExponentialGolomb(), 3);
    ASSERT_EQ(stream.GetExponentialGolomb(), 0);
    ASSERT_EQ(stream.GetExponentialGolomb(), 8);
}

TEST_F(BitStreamTestFixture, testCollectValue) {
    std::vector<bool> data(kDataSize);

    // 0
    data[0] = false;
    // 1
    data[1] = true;

    data[2] = false;
    data[3] = false;
    data[4] = false;
    // 5
    data[5] = true;
    data[6] = false;
    data[7] = true;

    data[8] = false;
    data[9] = true;
    // 1
    data[10] = false;
    data[11] = false;
    data[12] = true;

    BitStream stream(data.begin(), data.begin());

    stream.CollectValue("zero");
    stream.CollectValue("one");
    stream.SkipBits(3);
    stream.CollectValue("five", 3);
    stream.SkipBits(2);
    stream.CollectValue("one-three", 3);

    ASSERT_EQ(stream.GetValue("zero"), 0);
    ASSERT_EQ(stream.GetValue("one"), 1);
    ASSERT_EQ(stream.GetValue("five"), 5);
    ASSERT_EQ(stream.GetValue("one-three"), 1);
}

TEST_F(BitStreamTestFixture, testSkipExponentialGolombs) {
    std::vector<bool> data(kDataSize);

    // 0
    data[0] = false;
    // 1
    data[1] = true;

    // 0-3
    data[2] = false;
    data[3] = false;
    data[4] = false;
    // 1 - 4
    data[5] = false;
    data[6] = false;
    data[7] = false;
    data[8] = true;
    // 5
    data[9] = false;
    data[10] = false;
    data[11] = true;
    data[12] = true;
    data[13] = false;
    // 0
    data[14]  = true;
    // 4
    data[15] = false;
    data[16] = false;
    data[17] = true;
    data[18] = false;
    data[19] = true;
    // end
    data[20] = false;

    BitStream stream(data.begin(), data.begin());

    stream.CollectValue("zero");
    stream.SkipExponentialGolombs("zero", 3);
    stream.MarkPosition("not-skipped");
    ASSERT_EQ(stream.GetValue("not-skipped"), 1);
    stream.CollectValue("one");
    stream.CollectValue("zero-three", 3);
    stream.SkipExponentialGolombs("zero-three", 1);
    stream.MarkPosition("not-skipped-two");
    ASSERT_EQ(stream.GetValue("not-skipped-two"), 5);
    stream.CollectValue("one-four", 4);
    stream.SkipExponentialGolombs("one-four", 3);
    stream.MarkPosition("end");
    ASSERT_EQ(stream.GetValue("end"), 20);
    stream.SkipFalse();
}