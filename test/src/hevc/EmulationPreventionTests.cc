#include "BitArray.h"
#include "Emulation.h"
#include <gtest/gtest.h>

using lightdb::utility::BitArray;
using lightdb::RemoveEmulationPrevention;
using lightdb::AddEmulationPreventionAndMarker;
using lightdb::GetNalMarker;
using lightdb::bytestring;

class EmulationPreventionTestFixture : public testing::Test {
    public:

    bytestring three() {
        bytestring bytes(3);
        bytes[0] = static_cast<unsigned char>(0);
        bytes[1] = static_cast<unsigned char>(0);
        bytes[2] = static_cast<unsigned char>(3);
        return bytes;
    }

    bytestring one() {
        bytestring bytes(3);
        bytes[0] = static_cast<unsigned char>(0);
        bytes[1] = static_cast<unsigned char>(0);
        bytes[2] = static_cast<unsigned char>(1);
        return bytes;
    }

    bytestring zero() {
        bytestring bytes(3);
        bytes[0] = static_cast<unsigned char>(0);
        bytes[1] = static_cast<unsigned char>(0);
        bytes[2] = static_cast<unsigned char>(0);
        return bytes;
    }

    bytestring threeone() {
        bytestring bytes(2);
        bytes[0] = static_cast<unsigned char>(0);
        bytes[1] = static_cast<unsigned char>(3);
        return bytes;
    }

    bytestring twoone() {
        bytestring bytes(2);
        bytes[0] = static_cast<unsigned char>(0);
        bytes[1] = static_cast<unsigned char>(2);
        return bytes;
    }

    bytestring oneone() {
        bytestring bytes(2);
        bytes[0] = static_cast<unsigned char>(0);
        bytes[1] = static_cast<unsigned char>(1);
        return bytes;
    }

    bytestring zeroone() {
        bytestring bytes(2);
        bytes[0] = static_cast<unsigned char>(0);
        bytes[1] = static_cast<unsigned char>(0);
        return bytes;
    }

    bytestring other() {
        bytestring bytes(3);
        bytes[0] = static_cast<unsigned char>(0);
        bytes[1] = static_cast<unsigned char>(0);
        bytes[2] = static_cast<unsigned char>(5);
        return bytes;
    }

    bytestring random() {
        bytestring bytes(5);
        bytes[0] = static_cast<unsigned char>(5);
        bytes[1] = static_cast<unsigned char>(9);
        bytes[2] = static_cast<unsigned char>(2);
        bytes[3] = static_cast<unsigned char>(4);
        bytes[4] = static_cast<unsigned char>(6);
        return bytes;
    }

    unsigned char GetChar(int index, BitArray &array) {
        unsigned char test = 0u;
        for (auto i = index * CHAR_BIT; i < index * CHAR_BIT + CHAR_BIT; i++) {
            test = (test << 1u) | array[i];
        }
        return test;
    }

};

TEST_F(EmulationPreventionTestFixture, testConstructor) {
}

TEST_F(EmulationPreventionTestFixture, testEmulation) {
    bytestring bytes;

    auto three = EmulationPreventionTestFixture::three();
    auto threeone = EmulationPreventionTestFixture::threeone();
    auto twoone = EmulationPreventionTestFixture::twoone();
    auto oneone = EmulationPreventionTestFixture::oneone();
    auto zeroone = EmulationPreventionTestFixture::zeroone();
    auto random = EmulationPreventionTestFixture::random();
    auto other = EmulationPreventionTestFixture::other();

    bytes.insert(bytes.begin(), three.begin(), three.end());
    bytes.push_back(static_cast<unsigned char>(0));

    // Sequence is 0030030, should recognize both bytes
    bytes.insert(bytes.end(), threeone.begin(), threeone.end());
    bytes.push_back(static_cast<unsigned char>(0));

    // Now is 0030030032, should also recognize the third
    bytes.insert(bytes.end(), threeone.begin(), threeone.end());
    bytes.push_back(static_cast<unsigned char>(2));

    // 003003003202, should do nothing - 000000202
    bytes.insert(bytes.end(), twoone.begin(), twoone.end());

    // 00300300320201, should do nothing - 00000020201
    bytes.insert(bytes.end(), oneone.begin(), oneone.end());

    // 0030030032020100500, again nothing - 0000002020100500
    bytes.insert(bytes.end(), other.begin(), other.end());
    bytes.insert(bytes.end(), zeroone.begin(), zeroone.end());

    // 003003003202010050059246
    bytes.insert(bytes.end(), random.begin(), random.end());

    // 0030030032020100500592460033, should recognize it
    bytes.insert(bytes.end(), three.begin(), three.end());
    bytes.push_back(static_cast<unsigned char>(3));

    // 00300300320201005005924600330031, should also recognize this
    bytes.insert(bytes.end(), three.begin(), three.end());
    bytes.push_back(static_cast<unsigned char>(1));

    // 00300300320201005005924600330031005, should do nothing
    bytes.insert(bytes.end(), other.begin(), other.end());

    auto end_size = bytes.size();
    BitArray removed = RemoveEmulationPrevention(bytes, 0, bytes.size());
    ASSERT_EQ((end_size - 5) * CHAR_BIT, removed.size());
    bytestring new_bytes(end_size - 5);

    // 00300300320201005005924600330031005
    // 00
    for (auto i = 0; i < 2; i++) {
        new_bytes.push_back(GetChar(i, removed));
        ASSERT_EQ(new_bytes.back(), bytes[i]);
    }
    // 00300300320201005005924600330031005
    // 0000
    for (auto i = 2; i < 4; i++) {
        new_bytes.push_back(GetChar(i, removed));
        ASSERT_EQ(new_bytes.back(), bytes[i + 1]);
    }
    // 00300300320201005005924600330031005
    // 000000
    for (auto i = 4; i < 6; i++) {
        new_bytes.push_back(GetChar(i, removed));
        ASSERT_EQ(new_bytes.back(), bytes[i + 2]);
    }
    // 00300300320201005005924600330031005
    // 00000020201005005924600
    for (auto i = 6; i < 23; i++) {
        new_bytes.push_back(GetChar(i, removed));
        ASSERT_EQ(new_bytes.back(), bytes[i + 3]);
    }
    // 00300300320201005005924600330031005
    // 00000020201005005924600300
    for (auto i = 23; i < 26; i++) {
        new_bytes.push_back(GetChar(i, removed));
        ASSERT_EQ(new_bytes.back(), bytes[i + 4]);
    }
    // 00300300320201005005924600330031005
    // 000000202010050059246003001005
    for (auto i = 26; i < 30; i++) {
        new_bytes.push_back(GetChar(i, removed));
        ASSERT_EQ(new_bytes.back(), bytes[i + 5]);
    }

    bytestring added = AddEmulationPreventionAndMarker(removed, 0, removed.size() / 8);
    bytestring nal_marker = GetNalMarker();
    bytes.insert(bytes.begin(), nal_marker.begin(), nal_marker.end());
    ASSERT_EQ(added, bytes);
}

TEST_F(EmulationPreventionTestFixture, testEmulationOffset) {
    bytestring bytes;

    auto three = EmulationPreventionTestFixture::three();
    auto threeone = EmulationPreventionTestFixture::threeone();
    auto twoone = EmulationPreventionTestFixture::twoone();
    auto oneone = EmulationPreventionTestFixture::oneone();
    auto zeroone = EmulationPreventionTestFixture::zeroone();
    auto random = EmulationPreventionTestFixture::random();
    auto other = EmulationPreventionTestFixture::other();

    bytes.insert(bytes.begin(), three.begin(), three.end());
    bytes.push_back(static_cast<unsigned char>(0));

    // Sequence is 0030030, should recognize both bytes
    bytes.insert(bytes.end(), threeone.begin(), threeone.end());
    bytes.push_back(static_cast<unsigned char>(0));

    // Now is 0030030032, should also recognize the third
    bytes.insert(bytes.end(), threeone.begin(), threeone.end());
    bytes.push_back(static_cast<unsigned char>(2));

    // 003003003202, should do nothing - 000000202
    bytes.insert(bytes.end(), twoone.begin(), twoone.end());

    // 00300300320201, should do nothing - 00000020201
    bytes.insert(bytes.end(), oneone.begin(), oneone.end());

    // 0030030032020100500, again nothing - 0000002020100500
    bytes.insert(bytes.end(), other.begin(), other.end());
    bytes.insert(bytes.end(), zeroone.begin(), zeroone.end());

    // 003003003202010050059246
    bytes.insert(bytes.end(), random.begin(), random.end());

    // 0030030032020100500592460033, should recognize it
    bytes.insert(bytes.end(), three.begin(), three.end());
    bytes.push_back(static_cast<unsigned char>(3));

    // 00300300320201005005924600330031, should also recognize this
    bytes.insert(bytes.end(), three.begin(), three.end());
    bytes.push_back(static_cast<unsigned char>(1));

    // 00300300320201005005924600330031005, should do nothing
    bytes.insert(bytes.end(), other.begin(), other.end());

    auto end_size = bytes.size();
    BitArray removed = RemoveEmulationPrevention(bytes, 6, 28);
    ASSERT_EQ((end_size - 2) * CHAR_BIT, removed.size());
    bytestring new_bytes(end_size - 5);

    // 0030030032020100500592460031005
    // 00300300
    for (auto i = 0; i < 8; i++) {
        new_bytes.push_back(GetChar(i, removed));
        ASSERT_EQ(new_bytes.back(), bytes[i]);
    }

    // 00300300320201005005924600330031005
    // 00300300202010050059246003
    for (auto i = 8; i < 25; i++) {
        new_bytes.push_back(GetChar(i, removed));
        ASSERT_EQ(new_bytes.back(), bytes[i + 1]);
    }

    // 00300300320201005005924600330031005
    // 003003002020100500592460030031005
    for (auto i = 25; i < 32; i++) {
        new_bytes.push_back(GetChar(i, removed));
        ASSERT_EQ(new_bytes.back(), bytes[i + 2]);
    }

    bytestring added = AddEmulationPreventionAndMarker(removed, 6, 28);
    bytestring nal_marker = GetNalMarker();
    bytes.insert(bytes.begin(), nal_marker.begin(), nal_marker.end());
    ASSERT_EQ(added, bytes);
}