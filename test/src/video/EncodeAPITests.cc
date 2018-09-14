#include "EncodeAPI.h"
#include "VideoEncoder.h"
#include "EncodeBuffer.h"
#include <gtest/gtest.h>

class EncodeAPITestFixture : public testing::Test {
public:
    EncodeAPITestFixture()
      : context(0),
        configuration(Configuration{1920, 1080, 0, 0, 1024*1024, {30, 1}}, NV_ENC_HEVC, 30),
        lock(context),
        encoder(context, configuration, lock)
    { }

protected:
    GPUContext context;
    EncodeConfiguration configuration;
    VideoLock lock;
    VideoEncoder encoder;
};

TEST_F(EncodeAPITestFixture, testConstructor) {
}

TEST_F(EncodeAPITestFixture, testPresetGUIDs) {
    const auto* preset = "hq";
    auto preset_guid = encoder.api().GetPresetGUID(preset, NV_ENC_HEVC);

    ASSERT_EQ(preset_guid, NV_ENC_PRESET_HQ_GUID);

    ASSERT_EQ(encoder.api().ValidatePresetGUID(preset_guid, NV_ENC_HEVC), NV_ENC_SUCCESS);
}

TEST_F(EncodeAPITestFixture, testEncodeFrame) {
    {
        EncodeBuffer encodeBuffer(encoder);

        std::scoped_lock{encodeBuffer};
        EXPECT_EQ(encoder.api().NvEncEncodeFrame(&encodeBuffer, nullptr, NV_ENC_PIC_STRUCT_FRAME), NV_ENC_SUCCESS);
    }
}