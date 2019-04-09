#include "Configuration.h"

CUVIDDECODECREATEINFO DecodeConfiguration::AsCuvidCreateInfo(CUvideoctxlock lock,
                                                             const unsigned short int left,
                                                             const unsigned short int top) const {
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
        return CUVIDDECODECREATEINFO{
                .ulWidth = width,
                .ulHeight = height,
                .ulNumDecodeSurfaces = decode_surfaces,
                .CodecType = codec.cudaId().value(),
                .ChromaFormat = chroma_format,
                .ulCreationFlags = creation_flags,
                //.bitDepthMinus8 = 0,
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
                .target_rect = {},
                .Reserved2 = {0}
        };
}

unsigned int DecodeConfiguration::DefaultDecodeSurfaces() const {
    unsigned int decode_surfaces;

    if ((codec.cudaId() == cudaVideoCodec_H264) ||
        (codec.cudaId() == cudaVideoCodec_H264_SVC) ||
        (codec.cudaId() == cudaVideoCodec_H264_MVC)) {
        // Assume worst-case of 20 decode surfaces for H264
        decode_surfaces = 20;
    //} else if (codec.cudaId() == cudaVideoCodec_VP9) {
    //    decode_surfaces = 12;
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
