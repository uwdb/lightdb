#ifndef VISUALCLOUD_CONFIGURATION_H
#define VISUALCLOUD_CONFIGURATION_H

#include "EncodeAPI.h"
#include <string>

struct Configuration {
    unsigned int width;
    unsigned int height;
    unsigned int max_width;
    unsigned int max_height;
    unsigned int fps;
    unsigned int bitrate;
};

typedef struct Configuration DecodeConfiguration;

struct EncodeConfiguration: public Configuration
{
    unsigned int              codec;
    NV_ENC_BUFFER_FORMAT      inputFormat;
    GUID                      preset;
    unsigned int              gopLength;
    unsigned int              numB;
    NV_ENC_PIC_STRUCT         pictureStruct;

    // Video buffering verifier (VBV) parameters
    struct {
        unsigned int          maxBitrate;
        int                   size;
    } videoBufferingVerifier;

    // Quantization parameters
    struct {
        unsigned int     quantizationParameter;
        std::string      deltaMapFilename;
        NV_ENC_PARAMS_RC_MODE rateControlMode;

        // I frame initial quantization: qp * factor + offset
        float            i_quant_factor;
        float            i_quant_offset;

        // B frame initial quantization: qp * factor + offset
        float            b_quant_factor;
        float            b_quant_offset;
    } quantization;

    // How often should an IDR refresh frame be injected?
    // Ignored when GOP != NVENC_INFINITE_GOPLENGTH
    struct {
        bool enabled;
        unsigned int period;   // Number of frames to perform the refresh over
        unsigned int duration; // Period between successive refreshes
    } intraRefresh;

    struct {
        bool enableMEOnly;
        bool enableAsyncMode;
        bool enableTemporalAQ;
        bool enableReferenceFrameInvalidation;
    } flags;

    EncodeConfiguration(const struct EncodeConfiguration& copy) = default;

    EncodeConfiguration(const struct EncodeConfiguration& model, const size_t height, const size_t width)
            : EncodeConfiguration(model)
    {
        this->height = height;
        this->width = width;
        this->max_height = model.max_height != 0 && model.max_height < height ? height : model.max_height;
        this->max_width = model.max_width != 0 && model.max_width < width ? width : model.max_width;
    }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int codec, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, 0, 0, codec, NV_ENC_PRESET_DEFAULT_GUID,
                                fps, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int codec, std::string preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, 0, 0, codec, EncodeAPI::GetPresetGUID(preset.c_str(), codec),
                                fps, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int codec, GUID preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, 0, 0, codec, preset, fps, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const unsigned int codec, std::string preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, max_height, max_width, codec, EncodeAPI::GetPresetGUID(preset.c_str(), codec),
                                fps, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const unsigned int codec, GUID preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const unsigned int b_frames=0,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP,
                        const NV_ENC_BUFFER_FORMAT input_format=NV_ENC_BUFFER_FORMAT_NV12,
                        const NV_ENC_PIC_STRUCT picture_struct=NV_ENC_PIC_STRUCT_FRAME,
                        const unsigned int i_qfactor=DEFAULT_I_QFACTOR,
                        const unsigned int b_qfactor=DEFAULT_B_QFACTOR,
                        const unsigned int i_qoffset=DEFAULT_I_QOFFSET,
                        const unsigned int b_qoffset=DEFAULT_B_QOFFSET) :
            Configuration{width, height, max_width, max_height, fps, bitrate},
            codec(codec),
            inputFormat(input_format),
            preset(preset),
            gopLength(gop_length),
            numB(b_frames),
            pictureStruct(picture_struct),
            videoBufferingVerifier{0, 0},
            quantization{28, "", rateControlMode, i_qfactor, b_qfactor, i_qoffset, b_qoffset},
            intraRefresh{0, 0, 0},
            flags{false, false, false, false}
    { }
};

#endif //VISUALCLOUD_CONFIGURATION_H
