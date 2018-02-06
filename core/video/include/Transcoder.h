#ifndef LIGHTDB_TRANSCODER_H
#define LIGHTDB_TRANSCODER_H

#include "CropTranscoder.h"


class Transcoder: protected CropTranscoder {
public:
    Transcoder(GPUContext& context,
               DecodeConfiguration &decodeConfiguration,
               EncodeConfiguration &encodeConfiguration)
            : CropTranscoder(context, decodeConfiguration, encodeConfiguration)
    { }

    void transcode(DecodeReader &reader, EncodeWriter &writer) {
        transcode(reader, writer, std::optional<size_t>{});
    }

    void transcode(DecodeReader &reader, EncodeWriter &writer, std::optional<size_t> frames) {
        CropTranscoder::crop(reader, writer, 0, 0, frames);
    }

    void transcode(DecodeReader &reader, EncodeWriter &writer, std::vector<FrameTransform> transforms) {
        CropTranscoder::crop(reader, writer, transforms, 0, 0, std::optional<size_t>{});
    }

    void transcode(DecodeReader &reader, EncodeWriter &writer, std::vector<FrameTransform> transforms,
                   std::optional<size_t> frames) {
        CropTranscoder::crop(reader, writer, transforms, 0, 0, frames);
    }

    void transcode(DecodeReader &reader, EncodeWriter &writer, FrameTransform transform) {
        CropTranscoder::crop(reader, writer, transform, 0, 0, std::optional<size_t>{});
    }

    void transcode(DecodeReader &reader, EncodeWriter &writer, FrameTransform transform, std::optional<size_t> frames) {
        CropTranscoder::crop(reader, writer, transform, 0, 0, frames);
    }

    VideoEncoder &encoder() { return CropTranscoder::encoder(); }
    VideoDecoder &decoder() { return CropTranscoder::decoder(); }
};

#endif // LIGHTDB_TRANSCODER_H
