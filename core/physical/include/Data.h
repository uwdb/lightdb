#ifndef LIGHTDB_DATA_H
#define LIGHTDB_DATA_H

#include "Encoding.h"
#include "DecodeReader.h"
#include "Codec.h"
#include <utility>

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
        virtual const bytestring& value() const { return *value_; }

    protected:
        SerializableData(DeviceType device, bytestring &value)
            : SerializableData(device, value.begin(), value.end())
        { }

        template<typename Input>
        SerializableData(DeviceType device, Input begin, Input end)
                : Data(device), value_(std::make_unique<bytestring>(begin, end))
        { }

    private:
        const std::shared_ptr<bytestring> value_;
    };

    class EncodedFrameData: public SerializableData {
    protected:
        EncodedFrameData(const DeviceType device, const Codec &codec, const bytestring &value)
                : EncodedFrameData(device, codec, value.begin(), value.end())
        { }

        template<typename Input>
        EncodedFrameData(const DeviceType device, Codec codec, Input begin, const Input end)
                : SerializableData(device, begin, end), codec_(std::move(codec))
        { }

    public:
        const Codec& codec() const { return codec_; }

    private:
        const Codec codec_;
    };

    class CPUEncodedFrameData: public EncodedFrameData {
    public:
        explicit CPUEncodedFrameData(const Codec &codec, const std::vector<char> &data)
                : EncodedFrameData(DeviceType::CPU, codec, data.begin(), data.end()),
                  packet_(data)
        { }

        explicit CPUEncodedFrameData(const Codec &codec, const DecodeReaderPacket &packet)
                : EncodedFrameData(DeviceType::CPU, codec, packet.payload, packet.payload + packet.payload_size),
                  packet_(packet)
        { }

        explicit operator const DecodeReaderPacket() const noexcept { return packet_; }

    private:
        const DecodeReaderPacket packet_;
    };

    class GPUDecodedFrameData: public Data {
    public:
        explicit GPUDecodedFrameData(std::vector<DecodedFrame> frames)
                : Data(DeviceType::GPU), frames_{std::move(frames)}
        { }

        GPUDecodedFrameData(const GPUDecodedFrameData& other)
                : Data(DeviceType::GPU), frames_(other.frames_)
        { }

        GPUDecodedFrameData(GPUDecodedFrameData &&other) noexcept
                : Data(DeviceType::GPU), frames_{other.frames_}
        { }

        inline std::vector<DecodedFrame>& frames() noexcept { return frames_; }

    private:
        std::vector<DecodedFrame> frames_{};
    };
} // namespace lightdb::physical

#endif //LIGHTDB_DATA_H
