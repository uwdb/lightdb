#include "Frame.h"

std::shared_ptr<CudaFrame> DecodedFrame::cuda() const {
    return std::make_shared<CudaDecodedFrame>(*this);
}
