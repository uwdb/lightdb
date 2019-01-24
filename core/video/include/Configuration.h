#ifndef LIGHTDB_CONFIGURATION_H
#define LIGHTDB_CONFIGURATION_H

#include "EncodeAPI.h"
#include "VideoLock.h"
#include "Codec.h"
#include "number.h"
#include <string>

struct Configuration {
    unsigned int width;
    unsigned int height;
    //TODO maxes belong in EncodeConfiguration, not here
    unsigned int max_width;
    unsigned int max_height;
    //TODO bitrate does not belong here
    size_t bitrate;
    struct FrameRate: public lightdb::rational {
        explicit FrameRate(const lightdb::rational &rational)
                : lightdb::rational(rational)
        { }

        FrameRate(const unsigned int numerator, const unsigned int denominator)
                : lightdb::rational(numerator, denominator)
        { }

        lightdb::real_type fps() const { return (lightdb::real_type)*this; }
    } framerate;
    struct {
        unsigned int left, top;
    } offset;
};

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

    EncodeConfiguration(const struct EncodeConfiguration&) = default;

    EncodeConfiguration(const struct EncodeConfiguration& model, const unsigned int height, const unsigned int width)
            : EncodeConfiguration(model)
    {
        this->height = height;
        this->width = width;
        this->max_height = model.max_height != 0 && model.max_height < height ? height : model.max_height;
        this->max_width = model.max_width != 0 && model.max_width < width ? width : model.max_width;
    }

    EncodeConfiguration(const Configuration &configuration,
                        const EncodeCodec codec,
                        const unsigned int gop_length,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP,
                        const unsigned int b_frames=0,
                        const NV_ENC_BUFFER_FORMAT input_format=NV_ENC_BUFFER_FORMAT_NV12,
                        const NV_ENC_PIC_STRUCT picture_struct=NV_ENC_PIC_STRUCT_FRAME,
                        const float i_qfactor=DEFAULT_I_QFACTOR,
                        const float b_qfactor=DEFAULT_B_QFACTOR,
                        const float i_qoffset=DEFAULT_I_QOFFSET,
                        const float b_qoffset=DEFAULT_B_QOFFSET):
            EncodeConfiguration(configuration,
                                codec, NV_ENC_PRESET_DEFAULT_GUID,
                                //configuration.framerate,
                                gop_length, configuration.bitrate,
                                rateControlMode, b_frames, input_format, picture_struct,
                                i_qfactor, b_qfactor, i_qoffset, b_qoffset)
    { }

    EncodeConfiguration(const Configuration &configuration,
                        const EncodeCodec codec,
                        const std::string &preset,
                        const unsigned int gop_length,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP,
                        const unsigned int b_frames=0,
                        const NV_ENC_BUFFER_FORMAT input_format=NV_ENC_BUFFER_FORMAT_NV12,
                        const NV_ENC_PIC_STRUCT picture_struct=NV_ENC_PIC_STRUCT_FRAME,
                        const float i_qfactor=DEFAULT_I_QFACTOR,
                        const float b_qfactor=DEFAULT_B_QFACTOR,
                        const float i_qoffset=DEFAULT_I_QOFFSET,
                        const float b_qoffset=DEFAULT_B_QOFFSET):
            EncodeConfiguration(configuration,
                                codec, EncodeAPI::GetPresetGUID(preset.c_str(), codec),
                                //configuration.framerate,
                                gop_length, configuration.bitrate,
                                rateControlMode, b_frames, input_format, picture_struct,
                                i_qfactor, b_qfactor, i_qoffset, b_qoffset)
    { }

    EncodeConfiguration(const Configuration &configuration,
                        const EncodeCodec codec, GUID preset, //const Configuration::FrameRate &fps,
                        const unsigned int gop_length, const size_t bitrate,
                        const NV_ENC_PARAMS_RC_MODE rateControlMode=NV_ENC_PARAMS_RC_CONSTQP,
                        const unsigned int b_frames=0,
                        const NV_ENC_BUFFER_FORMAT input_format=NV_ENC_BUFFER_FORMAT_NV12,
                        const NV_ENC_PIC_STRUCT picture_struct=NV_ENC_PIC_STRUCT_FRAME,
                        const float i_qfactor=DEFAULT_I_QFACTOR,
                        const float b_qfactor=DEFAULT_B_QFACTOR,
                        const float i_qoffset=DEFAULT_I_QOFFSET,
                        const float b_qoffset=DEFAULT_B_QOFFSET) :
            Configuration{configuration},
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
    const lightdb::Codec codec;
    const cudaVideoChromaFormat chroma_format;
    const cudaVideoSurfaceFormat output_format;
    const unsigned long output_surfaces;
    const unsigned int decode_surfaces;
    const unsigned long creation_flags;
    const cudaVideoDeinterlaceMode deinterlace_mode;

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const lightdb::rational &fps,
                        const lightdb::Codec &codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 32,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration({width, height, max_width, max_height, 0, FrameRate{fps}, {0, 0}},
                                  codec, chroma_format, output_format, output_surfaces, decode_surfaces,
                                  creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int max_height, const unsigned int max_width,
                        const lightdb::rational &fps, const unsigned int bitrate,
                        const lightdb::Codec &codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 32,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration({width, height, max_width, max_height, 0, FrameRate{fps}, {0, 0}},
                                  codec, chroma_format, output_format, output_surfaces, decode_surfaces,
                                  creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const unsigned int fps, const unsigned int bitrate,
                        const lightdb::Codec &codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 32,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration(height, width, {fps, 1u}, bitrate, codec,
                                   chroma_format, output_format, output_surfaces, creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const lightdb::rational &fps, const unsigned int bitrate,
                        const lightdb::Codec &codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 32,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration(height, width, height, width, fps, bitrate, codec,
                                  chroma_format, output_format, output_surfaces, creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const unsigned int height, const unsigned int width,
                        const lightdb::rational &fps,
                        const lightdb::Codec &codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 32,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : DecodeConfiguration(height, width, height, width, fps, 0, codec,chroma_format, output_format, output_surfaces, creation_flags, deinterlace_mode)
    { }

    DecodeConfiguration(const Configuration &configuration,
                        lightdb::Codec codec,
                        const cudaVideoChromaFormat chroma_format = cudaVideoChromaFormat_420,
                        const cudaVideoSurfaceFormat output_format = cudaVideoSurfaceFormat_NV12,
                        const unsigned long output_surfaces = 32,
                        const unsigned int decode_surfaces = 0,
                        const unsigned long creation_flags = cudaVideoCreate_PreferCUVID,
                        const cudaVideoDeinterlaceMode deinterlace_mode = cudaVideoDeinterlaceMode_Weave)
            : Configuration{configuration},
              codec(std::move(codec)), chroma_format(chroma_format),
              output_format(output_format),
              output_surfaces(output_surfaces),
              decode_surfaces(decode_surfaces != 0 ? decode_surfaces : DefaultDecodeSurfaces()),
              creation_flags(creation_flags),
              deinterlace_mode(deinterlace_mode)
    { }

    DecodeConfiguration(DecodeConfiguration&&) = default;
    DecodeConfiguration(const DecodeConfiguration&) = default;

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
            throw InvalidArgumentError("Left must be less than frame width", "left");
        else if(top > height)
            throw InvalidArgumentError("Top must be less than frame height", "top");
        else if(!codec.cudaId().has_value())
            throw GpuCudaRuntimeError("Codec does not have a CUDA equivalent", CUDA_ERROR_INVALID_VALUE);
        else
            return {
                    .ulWidth = width,
                    .ulHeight = height,
                    .ulNumDecodeSurfaces = decode_surfaces,
                    .CodecType = codec.cudaId().value(),
                    .ChromaFormat = chroma_format,
                    .ulCreationFlags = creation_flags,
                    .bitDepthMinus8 = 0,
                    .Reserved1 = {0},
                    .display_area = {
                            .left = static_cast<short>(left),
                            .top = static_cast<short>(top),
                            .right = static_cast<short>(width - left),
                            .bottom = static_cast<short>(height - top),
                    },
                    .OutputFormat = output_format,
                    .DeinterlaceMode = deinterlace_mode,
                    .ulTargetWidth = width,
                    .ulTargetHeight = height,
                    .ulNumOutputSurfaces = output_surfaces,
                    .vidLock = lock,
                    .target_rect = {0, 0, 0, 0},
                    .Reserved2 = {0}
            };
    }

private:
    unsigned int DefaultDecodeSurfaces() const {
        unsigned int decode_surfaces;

        if ((codec.cudaId() == cudaVideoCodec_H264) ||
            (codec.cudaId() == cudaVideoCodec_H264_SVC) ||
            (codec.cudaId() == cudaVideoCodec_H264_MVC)) {
            // Assume worst-case of 20 decode surfaces for H264
            decode_surfaces = 20;
        } else if (codec.cudaId() == cudaVideoCodec_VP9) {
            decode_surfaces = 12;
        } else if (codec.cudaId() == cudaVideoCodec_HEVC) {
            // ref HEVC spec: A.4.1 General tier and level limits
            auto maxLumaPS = 35651584u; // currently assuming level 6.2, 8Kx4K
            auto maxDpbPicBuf = 6u;
            auto picSizeInSamplesY = width * height;
            unsigned int MaxDpbSize;
            if (picSizeInSamplesY <= (maxLumaPS >> 2))
                MaxDpbSize = maxDpbPicBuf * 4;
            else if (picSizeInSamplesY <= (maxLumaPS >> 1))
                MaxDpbSize = maxDpbPicBuf * 2;
            else if (picSizeInSamplesY <= ((3 * maxLumaPS) >> 2))
                MaxDpbSize = (maxDpbPicBuf * 4) / 3;
            else
                MaxDpbSize = maxDpbPicBuf;
            MaxDpbSize = MaxDpbSize < 16 ? MaxDpbSize : 16;
            decode_surfaces = MaxDpbSize + 4;
        } else {
            decode_surfaces = 8;
        }

        return decode_surfaces;
    }
};

#endif //LIGHTDB_CONFIGURATION_H
