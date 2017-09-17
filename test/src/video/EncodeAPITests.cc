#include "EncodeAPI.h"
#include "VideoEncoder.h"
#include "EncodeBuffer.h"
#include <gtest/gtest.h>

class EncodeAPITestFixture : public testing::Test {
public:
    EncodeAPITestFixture()
      : context(0), encodeAPI(context)
    { }

protected:
    GPUContext context;
    EncodeAPI encodeAPI;
};

TEST_F(EncodeAPITestFixture, testConstructor) {
}

TEST_F(EncodeAPITestFixture, testPresetGUIDs) {
    const auto* preset = "hq";
    auto preset_guid = encodeAPI.GetPresetGUID(preset, NV_ENC_HEVC);

    ASSERT_EQ(preset_guid, NV_ENC_PRESET_HQ_GUID);

    ASSERT_EQ(encodeAPI.ValidatePresetGUID(preset_guid, NV_ENC_HEVC), NV_ENC_SUCCESS);
}

TEST_F(EncodeAPITestFixture, testCreateEncoder) {
    EncodeConfiguration configuration(1080, 1920, NV_ENC_HEVC, 30, 30, 1024*1024);

    ASSERT_EQ(encodeAPI.CreateEncoder(&configuration), NV_ENC_SUCCESS);
}

TEST_F(EncodeAPITestFixture, testConstructWithoutEncoder) {
    EncodeConfiguration configuration(1080, 1920, NV_ENC_HEVC, 30, 30, 1024*1024);
    ASSERT_ANY_THROW(EncodeBuffer(encodeAPI, configuration));
}

TEST_F(EncodeAPITestFixture, testEncodeFrame) {
    EncodeConfiguration configuration(1080, 1920, NV_ENC_HEVC, 30, 30, 1024*1024);

    ASSERT_EQ(encodeAPI.CreateEncoder(&configuration), NV_ENC_SUCCESS);

    {
        EncodeBuffer encodeBuffer(encodeAPI, configuration);

        std::scoped_lock{encodeBuffer};
        EXPECT_EQ(encodeAPI.NvEncEncodeFrame(&encodeBuffer, nullptr, NV_ENC_PIC_STRUCT_FRAME), NV_ENC_SUCCESS);
    }

    EXPECT_EQ(encodeAPI.NvEncDestroyEncoder(), NV_ENC_SUCCESS);
}