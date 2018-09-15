//
// Created by sophi on 5/18/2018.
//

#include <climits>
#include <vector>

#include <gtest/gtest.h>

#include "BitArray.h"
#include "Golombs.h"

using lightdb::utility::BitArray;
using lightdb::EncodeGolombs;
using std::vector;

class GolombEncodeTestFixture : public testing::Test {
public:

    GolombEncodeTestFixture() = default;

};

TEST_F(GolombEncodeTestFixture, testConstructor) {
}

TEST_F(GolombEncodeTestFixture, testEncodeGolombs) {
    vector<unsigned long> golombs(5);
    golombs[0] = 3;
    golombs[1] = 0;
    golombs[2] = 298;
    golombs[3] = 1;
    golombs[4] = 12;
    BitArray encoded = EncodeGolombs(golombs);
    ASSERT_EQ(encoded.size(), 33);
    ASSERT_FALSE(encoded[0]);
    ASSERT_FALSE(encoded[1]);
    ASSERT_TRUE(encoded[2]);
    ASSERT_FALSE(encoded[3]);
    ASSERT_FALSE(encoded[4]);
    ASSERT_TRUE(encoded[5]);
    ASSERT_FALSE(encoded[6]);
    ASSERT_FALSE(encoded[7]);
    ASSERT_FALSE(encoded[8]);
    ASSERT_FALSE(encoded[9]);
    ASSERT_FALSE(encoded[10]);
    ASSERT_FALSE(encoded[11]);
    ASSERT_FALSE(encoded[12]);
    ASSERT_FALSE(encoded[13]);
    ASSERT_TRUE(encoded[14]);
    ASSERT_FALSE(encoded[15]);
    ASSERT_FALSE(encoded[16]);
    ASSERT_TRUE(encoded[17]);
    ASSERT_FALSE(encoded[18]);
    ASSERT_TRUE(encoded[19]);
    ASSERT_FALSE(encoded[20]);
    ASSERT_TRUE(encoded[21]);
    ASSERT_TRUE(encoded[22]);
    ASSERT_FALSE(encoded[23]);
    ASSERT_TRUE(encoded[24]);
    ASSERT_FALSE(encoded[25]);
    ASSERT_FALSE(encoded[26]);
    ASSERT_FALSE(encoded[27]);
    ASSERT_FALSE(encoded[28]);
    ASSERT_TRUE(encoded[29]);
    ASSERT_TRUE(encoded[30]);
    ASSERT_FALSE(encoded[31]);
    ASSERT_TRUE(encoded[32]);
}




