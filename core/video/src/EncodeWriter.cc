#include "EncodeWriter.h"

//TODO move these into EncodeWriter.h, deal with empty implementation below
NVENCSTATUS EncodeWriter::WriteFrame(const EncodeBuffer &buffer) {
    NVENCSTATUS result;
    NV_ENC_LOCK_BITSTREAM bitstream{0};
    bitstream.version = NV_ENC_LOCK_BITSTREAM_VER;
    bitstream.outputBitstream = buffer.stOutputBfr.hBitstreamBuffer;
    bitstream.doNotWait = 0;

    if (buffer.stOutputBfr.hBitstreamBuffer == nullptr && !buffer.stOutputBfr.bEOSFlag) {
        result = NV_ENC_ERR_INVALID_PARAM;
    } else if (buffer.stOutputBfr.bWaitOnEvent && buffer.stOutputBfr.hOutputEvent != nullptr) {
        result = NV_ENC_ERR_INVALID_PARAM;
    } else if (buffer.stOutputBfr.bEOSFlag) {
        result = NV_ENC_SUCCESS;
    } else {
        result = WriteFrame(bitstream) == 0 ? NV_ENC_SUCCESS : NV_ENC_ERR_GENERIC;
    }

    return result;
}

NVENCSTATUS EncodeWriter::WriteFrame(NV_ENC_LOCK_BITSTREAM &bitstream) {
    NVENCSTATUS status;

    if((status = api.NvEncLockBitstream(&bitstream)) != NV_ENC_SUCCESS) {
        return status;
    }

    status = WriteFrame(bitstream.bitstreamBufferPtr, bitstream.bitstreamSizeInBytes) == 0
             ? NV_ENC_SUCCESS : NV_ENC_ERR_GENERIC;

    auto unlockStatus = api.NvEncUnlockBitstream(bitstream.outputBitstream);

    return status != NV_ENC_SUCCESS ? status : unlockStatus;
}

//NVENCSTATUS EncodeWriter::WriteFrame(const MotionEstimationBuffer &buffer) {
//}
