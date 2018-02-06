#ifndef LIGHTDB_CONFIGURATION_H
#define LIGHTDB_CONFIGURATION_H

#include "EncodeAPI.h"
#include "VideoLock.h"
#include "rational.h"
#include <string>

struct Configuration {
    unsigned int width;
    unsigned int height;
    unsigned int max_width;
    unsigned int max_height;
    size_t bitrate;
    struct FrameRate { //TODO change to type rational
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
                        const EncodeCodec codec, const lightdb::rational fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, 0, 0, codec, NV_ENC_PRESET_DEFAULT_GUID,
                                {fps.numerator(), fps.denominator()}, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const EncodeCodec codec, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, 0, 0, codec, NV_ENC_PRESET_DEFAULT_GUID,
                                {fps, 1}, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const EncodeCodec codec, std::string preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, 0, 0, codec, EncodeAPI::GetPresetGUID(preset.c_str(), codec),
                                {fps, 1}, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const EncodeCodec codec, GUID preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, 0, 0, codec, preset, {fps, 1}, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const EncodeCodec codec, std::string preset, const unsigned int fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP) :
            EncodeConfiguration(height, width, max_height, max_width, codec,
                                EncodeAPI::GetPresetGUID(preset.c_str(), codec),
                                {fps, 1}, gop_length, bitrate, rateControlMode)
    { }

    EncodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const EncodeCodec codec, GUID preset, const Configuration::FrameRate fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP,
                        const unsigned int b_frames=0,
                        const NV_ENC_BUFFER_FORMAT input_format=NV_ENC_BUFFER_FORMAT_NV12,
                        const NV_ENC_PIC_STRUCT picture_struct=NV_ENC_PIC_STRUCT_FRAME,
                        const float i_qfactor=DEFAULT_I_QFACTOR,
                        const float b_qfactor=DEFAULT_B_QFACTOR,
                        const float i_qoffset=DEFAULT_I_QOFFSET,
                        const float b_qoffset=DEFAULT_B_QOFFSET) :
            Configuration{width, height, max_width, max_height, bitrate, fps},
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
    unsigned int decode_surfaces;
    unsigned long creation_flags;
    cudaVideoDeinterlaceMode deinterlace_mode;

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const lightdb::rational fps,
                        const cudaVideoCodec codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 2,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration({height, width, max_height, max_width, 0, {fps.numerator(), fps.denominator()}},
                                  codec, chroma_format, output_format, output_surfaces, decode_surfaces,
                                  creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const lightdb::rational fps, const unsigned int bitrate,
                        const cudaVideoCodec codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 2,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration({height, width, max_height, max_width, 0, {fps.numerator(), fps.denominator()}},
                                  codec, chroma_format, output_format, output_surfaces, decode_surfaces,
                                  creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int fps, const unsigned int bitrate,
                        const cudaVideoCodec codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 2,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration(height, width, {fps, 1}, bitrate, codec,
                                   chroma_format, output_format, output_surfaces, creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const lightdb::rational fps, const unsigned int bitrate,
                        const cudaVideoCodec codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 2,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration(height, width, height, width, fps, bitrate, codec,
                                  chroma_format, output_format, output_surfaces, creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const lightdb::rational fps,
                        const cudaVideoCodec codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 2,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration(height, width, height, width, fps, 0, codec,
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
                                  output_format, output_surfaces, decode_surfaces,
                                  creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const Configuration &configuration,
                        const cudaVideoCodec codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 2,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : Configuration{configuration},
              codec(codec), chroma_format(chroma_format),
              output_format(output_format),
              output_surfaces(output_surfaces),
              decode_surfaces(decode_surfaces != 0 ? decode_surfaces : DefaultDecodeSurfaces()),
              creation_flags(creation_flags),
              deinterlace_mode(deinterlace_mode)
    { }

    CUVIDDECODECREATEINFO AsCuvidCreateInfo(VideoLock &lock, const unsigned int left = 0, const unsigned int top = 0) const {
        return AsCuvidCreateInfo(lock.get());
    }

    CUVIDDECODECREATEINFO AsCuvidCreateInfo(CUvideoctxlock lock,
                                            const unsigned short int left = 0,
                                            const unsigned short int top = 0) const {
        assert(left < SHRT_MAX);
        assert(top < SHRT_MAX);
        assert(width < SHRT_MAX);
        assert(height < SHRT_MAX);
        if(left > width)
            throw std::runtime_error("bad value"); //TODO
        if(top > height)
            throw std::runtime_error("bad value"); //TODO
        else
            return {
                    .ulWidth = width,
                    .ulHeight = height,
                    .ulNumDecodeSurfaces = decode_surfaces,
                    .CodecType = codec,
                    .ChromaFormat = chroma_format,
                    .ulCreationFlags = creation_flags,
                    .bitDepthMinus8 = 0,
                    .Reserved1 = {0},
                    .display_area = {
                            .left = static_cast<short>(left),
                            .top = static_cast<short>(top),
                            .right = static_cast<short>(width - left),
                            .bottom = static_cast<const short>(height - top),
                    },
                    .OutputFormat = output_format,
                    .DeinterlaceMode = deinterlace_mode,
                    .ulTargetWidth = width,
                    .ulTargetHeight = height,
                    .ulNumOutputSurfaces = output_surfaces,
                    .vidLock = lock,
                    .target_rect = {0},
                    .Reserved2 = {0}
            };
    }

private:
    unsigned int DefaultDecodeSurfaces() const {
        unsigned long decode_surfaces;

        if ((codec == cudaVideoCodec_H264) ||
            (codec == cudaVideoCodec_H264_SVC) ||
            (codec == cudaVideoCodec_H264_MVC)) {
            // Assume worst-case of 20 decode surfaces for H264
            decode_surfaces = 20;
        } else if (codec == cudaVideoCodec_VP9) {
            decode_surfaces = 12;
        } else if (codec == cudaVideoCodec_HEVC) {
            // ref HEVC spec: A.4.1 General tier and level limits
            auto MaxLumaPS = 35651584u; // currently assuming level 6.2, 8Kx4K
            auto MaxDpbPicBuf = 6;
            auto PicSizeInSamplesY = width * height;
            unsigned int MaxDpbSize;
            if (PicSizeInSamplesY <= (MaxLumaPS >> 2))
                MaxDpbSize = MaxDpbPicBuf * 4;
            else if (PicSizeInSamplesY <= (MaxLumaPS >> 1))
                MaxDpbSize = MaxDpbPicBuf * 2;
            else if (PicSizeInSamplesY <= ((3 * MaxLumaPS) >> 2))
                MaxDpbSize = (MaxDpbPicBuf * 4) / 3;
            else
                MaxDpbSize = MaxDpbPicBuf;
            MaxDpbSize = MaxDpbSize < 16 ? MaxDpbSize : 16;
            decode_surfaces = MaxDpbSize + 4;
        } else {
            decode_surfaces = 8;
        }

        return decode_surfaces;
    }
};

#endif //LIGHTDB_CONFIGURATION_H
