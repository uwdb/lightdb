#include "EncodeWriter.h"

NVENCSTATUS EncodeWriter::WriteFrame(const EncodeBuffer &buffer) {
    NVENCSTATUS status;

    NV_ENC_LOCK_BITSTREAM bitstream{0};
    bitstream.version = NV_ENC_LOCK_BITSTREAM_VER;
    bitstream.outputBitstream = buffer.stOutputBfr.hBitstreamBuffer;
    bitstream.doNotWait = false;

    if (buffer.stOutputBfr.hBitstreamBuffer == nullptr && !buffer.stOutputBfr.bEOSFlag)
        return NV_ENC_ERR_INVALID_PARAM;
    else if (buffer.stOutputBfr.bWaitOnEvent && !buffer.stOutputBfr.hOutputEvent)
        return NV_ENC_ERR_INVALID_PARAM;
    else if (buffer.stOutputBfr.bEOSFlag)
        return NV_ENC_SUCCESS;
    else
        return WriteFrame(bitstream) == 0 ? NV_ENC_SUCCESS : NV_ENC_ERR_GENERIC;
}

NVENCSTATUS EncodeWriter::WriteFrame(NV_ENC_LOCK_BITSTREAM &bitstream) {
    NVENCSTATUS status;

    if((status = api.NvEncLockBitstream(&bitstream)) != NV_ENC_SUCCESS)
        return status;

    status = WriteFrame(bitstream.bitstreamBufferPtr, bitstream.bitstreamSizeInBytes) == 0
             ? NV_ENC_SUCCESS : NV_ENC_ERR_GENERIC;

    auto unlockStatus = api.NvEncUnlockBitstream(bitstream.outputBitstream);

    return status != NV_ENC_SUCCESS ? status : unlockStatus;
}

NVENCSTATUS EncodeWriter::WriteFrame(const MotionEstimationBuffer &buffer) {

}
