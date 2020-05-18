#include "EncodeAPI.h"
#include "VideoEncoder.h"
#include "EncodeBuffer.h"
#include "Environment.h"
#include "RequiresGPUTest.h"
#include "nvUtils.h"

using lightdb::lazy;

class EncodeAPITestFixture : public RequiresGPUTest {
public:
    EncodeAPITestFixture()
      : configuration(Configuration{1920, 1080, 0, 0, 1024*1024, {30, 1}, {0, 0}}, NV_ENC_HEVC, 30),
        encoder([this]{ return VideoEncoder(context, configuration, lock); })
    { }

protected:
    EncodeConfiguration configuration;
    lazy<VideoEncoder> encoder;
};

TEST_F(EncodeAPITestFixture, testConstructor) {
}

TEST_F(EncodeAPITestFixture, testPresetGUIDs) {
    const auto* preset = "hq";
    auto preset_guid = EncodeAPI::GetPresetGUID(preset, NV_ENC_HEVC);

    ASSERT_EQ(preset_guid, NV_ENC_PRESET_HQ_GUID);

    ASSERT_EQ(encoder->api().ValidatePresetGUID(preset_guid, NV_ENC_HEVC), NV_ENC_SUCCESS);
}

TEST_F(EncodeAPITestFixture, testEncodeFrame) {
    {
        EncodeBuffer encodeBuffer(encoder);

        std::scoped_lock lock{encodeBuffer};
        EXPECT_EQ(encoder->api().NvEncEncodeFrame(&encodeBuffer, nullptr, NV_ENC_PIC_STRUCT_FRAME), NV_ENC_SUCCESS);
    }
}