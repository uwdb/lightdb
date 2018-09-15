//
// Created by sophi on 5/14/2018.
//

#include <climits>
#include <cmath>
#include <vector>

#include "BitStream.h"
#include <gtest/gtest.h>

using lightdb::utility::BitStream;
static const int kDataSize = 50;

class BitStreamTestFixture : public testing::Test {
public:

    BitStreamTestFixture() = default;

};

TEST_F(BitStreamTestFixture, testMarkPositionSkipBits) {
    std::vector<bool> data(kDataSize, 0);
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
    std::vector<bool> data(kDataSize, 0);
    for (auto i = 20; i < 25; i++) {
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
    data[0] = 0;
    data[1] = 0;
    data[2] = 1;
    data[3] = 0;
    data[4] = 0;
    // 2
    data[5] = 0;
    data[6] = 1;
    data[7] = 1;
    // 9 bits to skip
    data[8] = 0;
    data[9] = 0;
    data[10] = 0;
    data[11] = 0;
    data[12] = 0;
    data[13] = 0;
    data[14] = 0;
    data[15] = 0;
    data[16] = 0;
    // Mark bit
    data[17] = 1;
    // 0
    data[18] = 1;
    // shouldn't read this next value
    data[19] = 0;
    data[20] = 1;
    data[21] = 1;
    // shouldn't process this either
    data[22] = 0;
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
    data[0] = 0;
    data[1] = 0;
    data[2] = 1;
    data[3] = 0;
    data[4] = 0;
    // 0
    data[5] = 1;
    // 8
    data[6] = 0;
    data[7] = 0;
    data[8] = 0;
    data[9] = 1;
    data[10] = 0;
    data[11] = 0;
    data[12] = 1;
    BitStream stream(data.begin(), data.begin());
    ASSERT_EQ(stream.GetExponentialGolomb(), 3);
    ASSERT_EQ(stream.GetExponentialGolomb(), 0);
    ASSERT_EQ(stream.GetExponentialGolomb(), 8);
}

TEST_F(BitStreamTestFixture, testCollectValue) {
    std::vector<bool> data(kDataSize);
    // 0
    data[0] = 0;
    // 1
    data[1] = 1;

    data[2] = 0;
    data[3] = 0;
    data[4] = 0;
    // 5
    data[5] = 1;
    data[6] = 0;
    data[7] = 1;

    data[8] = 0;
    data[9] = 1;
    // 1
    data[10] = 0;
    data[11] = 0;
    data[12] = 1;
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
    data[0] = 0;
    // 1
    data[1] = 1;

    // 0-3
    data[2] = 0;
    data[3] = 0;
    data[4] = 0;
    // 1 - 4
    data[5] = 0;
    data[6] = 0;
    data[7] = 0;
    data[8] = 1;
    // 5
    data[9] = 0;
    data[10] = 0;
    data[11] = 1;
    data[12] = 1;
    data[13] = 0;
    // 0
    data[14]  = 1;
    // 4
    data[15] = 0;
    data[16] = 0;
    data[17] = 1;
    data[18] = 0;
    data[19] = 1;
    // end
    data[20] = 0;

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


