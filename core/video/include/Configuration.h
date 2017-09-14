#ifndef VISUALCLOUD_CONFIGURATION_H
#define VISUALCLOUD_CONFIGURATION_H

#include "EncodeAPI.h"
#include <string>

struct Configuration {
    unsigned int width;
    unsigned int height;
    unsigned int max_width;
    unsigned int max_height;
    unsigned int bitrate;
    struct FrameRate {
        unsigned int numerator;
        unsigned int denominator;

        float fps() const { return ((float)numerator) / denominator; }
    } framerate;
};

//typedef struct Configuration DecodeConfiguration;

struct EncodeConfiguration: public Configuration
{
    EncodeCodec               codec;
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
                        const EncodeCodec codec, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, 0, 0, codec, NV_ENC_PRESET_DEFAULT_GUID,
                                fps, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const EncodeCodec codec, std::string preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, 0, 0, codec, EncodeAPI::GetPresetGUID(preset.c_str(), codec),
                                fps, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const EncodeCodec codec, GUID preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, 0, 0, codec, preset, fps, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const EncodeCodec codec, std::string preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, max_height, max_width, codec,
                                EncodeAPI::GetPresetGUID(preset.c_str(), codec),
                                fps, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const EncodeCodec codec, GUID preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const unsigned int b_frames=0,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP,
                        const NV_ENC_BUFFER_FORMAT input_format=NV_ENC_BUFFER_FORMAT_NV12,
                        const NV_ENC_PIC_STRUCT picture_struct=NV_ENC_PIC_STRUCT_FRAME,
                        const unsigned int i_qfactor=DEFAULT_I_QFACTOR,
                        const unsigned int b_qfactor=DEFAULT_B_QFACTOR,
                        const unsigned int i_qoffset=DEFAULT_I_QOFFSET,
                        const unsigned int b_qoffset=DEFAULT_B_QOFFSET) :
            Configuration{width, height, max_width, max_height, bitrate, {fps, 1}},
            codec(codec),
            inputFormat(input_format),
            preset(preset),
            gopLength(gop_length),
            numB(b_frames),
            pictureStruct(picture_struct),
            videoBufferingVerifier{0, 0},
            quantization{28, "", rateControlMode, i_qfactor, b_qfactor, i_qoffset, b_qoffset},
            intraRefresh{false, 0, 0},
            flags{false, false, false, false}
    { }
};

struct DecodeConfiguration: public Configuration {
    cudaVideoCodec codec;
    cudaVideoChromaFormat chroma_format;
    cudaVideoSurfaceFormat output_format;
    unsigned long output_surfaces;
    unsigned long creation_flags;
    cudaVideoDeinterlaceMode deinterlace_mode;

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const unsigned int fps, const unsigned int bitrate,
                        const cudaVideoCodec codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 2,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration({height, width, height, width, fps, bitrate},
                                  codec, chroma_format, output_format, output_surfaces,
                                  creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int fps, const unsigned int bitrate,
                        const cudaVideoCodec codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 2,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration(height, width, height, width, fps, bitrate, codec,
                                   chroma_format, output_format, output_surfaces, creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const CUVIDEOFORMAT &video_format,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 2,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration({video_format.coded_width, video_format.coded_height,
                                   video_format.coded_width, video_format.coded_height, video_format.bitrate,
                                   {video_format.frame_rate.numerator, video_format.frame_rate.denominator} },
                                  video_format.codec,
                                  video_format.chroma_format,
                                  output_format, output_surfaces, creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const Configuration &configuration,
                        const cudaVideoCodec codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 2,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : Configuration{configuration},
              codec(codec), chroma_format(chroma_format),
              output_format(output_format),
              output_surfaces(output_surfaces), creation_flags(creation_flags),
              deinterlace_mode(deinterlace_mode)
    { }
};

#endif //VISUALCLOUD_CONFIGURATION_H
