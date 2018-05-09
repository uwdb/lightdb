#ifndef LIGHTDB_DATA_H
#define LIGHTDB_DATA_H

#include <utility>
#include <DecodeReader.h>

#include "Encoding.h"

namespace lightdb::physical {
    class Data;
    using DataReference = shared_reference<Data>;

    enum DeviceType {
        CPU,
        GPU,
        FPGA
    };

    class Data {
    public:
        Data(Data &) = default;
        Data(const Data &) = default;
        Data(Data &&) = default;

        virtual ~Data() = default;

        Data& operator=(const Data&) = default;
        Data& operator=(Data&&)      = default;

    protected:
        explicit Data(DeviceType device)
                : device_(device)
        { }

    private:
        DeviceType device_;
    };

    class SerializableData: public Data {
    public:
        virtual const bytestring& value() = 0;

    protected:
        explicit SerializableData(DeviceType device)
                : Data(device)
        { }
        SerializableData(DeviceType device, bytestring value)
            : Data(device), value_(std::move(value))
        { }

    protected:
        bytestring value_;
    };

    class CPUEncodedFrameData: public SerializableData {
    public:
        explicit CPUEncodedFrameData(const DecodeReaderPacket &packet)
                : SerializableData(DeviceType::CPU), packet_(packet)
        { }

        size_t size() const noexcept { return packet_.payload_size; }
        const void *data() const noexcept { return packet_.payload;}

        explicit operator const DecodeReaderPacket() const noexcept { return packet_; }

        const bytestring& value() override {
            return (value_ = bytestring(static_cast<const char*>(data()), static_cast<const char*>(data()) + size()));
        }

    private:
        const DecodeReaderPacket packet_;
    };

    class GPUDecodedFrameData: public Data {
    public:
        explicit GPUDecodedFrameData(std::vector<DecodedFrame> frames)
                : Data(DeviceType::GPU), frames_{std::move(frames)}
        { }

        inline std::vector<DecodedFrame>& frames() noexcept { return frames_; }

    private:
        std::vector<DecodedFrame> frames_;
    };
} // namespace lightdb::physical

#endif //LIGHTDB_DATA_H
