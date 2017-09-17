#include "VideoEncoder.h"
#include "EncodeBuffer.h"

std::vector<std::shared_ptr<EncodeBuffer>> VideoEncoder::CreateBuffers(const size_t size) {
    std::vector<std::shared_ptr<EncodeBuffer>> buffers;

    buffers.reserve(size);
    std::generate_n(std::back_inserter(buffers), size,
                    [this]() { return std::make_shared<EncodeBuffer>(this->api(), this->configuration()); });
    return buffers;
}