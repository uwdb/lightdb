#include "VideoEncoder.h"
#include "EncodeBuffer.h"

std::vector<std::shared_ptr<EncodeBuffer>> VideoEncoder::CreateBuffers(
        const size_t size, const NV_ENC_BUFFER_FORMAT format) {
    std::vector<std::shared_ptr<EncodeBuffer>> buffers;

    buffers.reserve(size);
    std::generate_n(std::back_inserter(buffers), size,
                    [this, &format]() { return std::make_shared<EncodeBuffer>(*this, format); });
    return buffers;
}