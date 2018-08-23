#ifndef LIGHTDB_DATA_H
#define LIGHTDB_DATA_H

#include "Encoding.h"
#include "DecodeReader.h"
#include "Frame.h"
#include "Codec.h"
#include <utility>

namespace lightdb::physical {
    class MaterializedLightField;
    using MaterializedLightFieldReference = shared_reference<MaterializedLightField>;

    enum DeviceType {
        CPU,
        GPU,
        FPGA
    };

    class MaterializedLightField: public LightField {
    public:
        MaterializedLightField(MaterializedLightField &) = default;
        MaterializedLightField(const MaterializedLightField &) = default;
        MaterializedLightField(MaterializedLightField &&) = default;

        ~MaterializedLightField() override = default;

        MaterializedLightField& operator=(const MaterializedLightField&) = default;
        MaterializedLightField& operator=(MaterializedLightField&&)      = default;

        explicit virtual operator MaterializedLightFieldReference() = 0;
        void accept(LightFieldVisitor& visitor) override { visitor.visit(*this); }

        DeviceType device() const { return device_; }

    protected:
        explicit MaterializedLightField(DeviceType device)
                //TODO remove hardcoded placeholders
                : LightField({}, Volume::limits(), UnknownColorSpace::instance()),
                  device_(device)
        { }

    private:
        DeviceType device_;
    };

    class SerializableData: public MaterializedLightField {
    public:
        virtual const bytestring& value() const { return *value_; }

    protected:
        SerializableData(DeviceType device, bytestring &value)
            : SerializableData(device, value.begin(), value.end())
        { }

        template<typename Input>
        SerializableData(DeviceType device, Input begin, Input end)
                : MaterializedLightField(device), value_(std::make_unique<bytestring>(begin, end))
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

        inline explicit operator const DecodeReaderPacket() const noexcept { return packet_; }
        inline explicit operator MaterializedLightFieldReference() override { return MaterializedLightFieldReference::make<CPUEncodedFrameData>(*this); }

    private:
        const DecodeReaderPacket packet_;
    };

    class CPUDecodedFrameData: public MaterializedLightField {
    public:
        CPUDecodedFrameData()
                : MaterializedLightField(DeviceType::CPU),
                  frames_{}
        { }

        explicit CPUDecodedFrameData(std::vector<LocalFrameReference> frames)
                : MaterializedLightField(DeviceType::CPU),
                  frames_(std::move(frames))
        { }

        CPUDecodedFrameData(const CPUDecodedFrameData& other)
                : MaterializedLightField(DeviceType::CPU),
                  frames_(other.frames_)
        { }

        CPUDecodedFrameData(CPUDecodedFrameData &&other) noexcept
                : MaterializedLightField(DeviceType::CPU),
                  frames_{std::move(other.frames_)}
        { }

        inline std::vector<LocalFrameReference>& frames() noexcept { return frames_; }
        inline explicit operator std::vector<LocalFrameReference>&() noexcept { return frames_; }
        inline explicit operator MaterializedLightFieldReference() override { return MaterializedLightFieldReference::make<CPUDecodedFrameData>(*this); }

    private:
        std::vector<LocalFrameReference> frames_{};
    };

    class GPUDecodedFrameData: public MaterializedLightField {
    public:
        GPUDecodedFrameData()
                : MaterializedLightField(DeviceType::GPU),
                  frames_{}
        { }

        explicit GPUDecodedFrameData(std::vector<GPUFrameReference> frames)
                : MaterializedLightField(DeviceType::GPU),
                  frames_(std::move(frames))
        { }

        GPUDecodedFrameData(const GPUDecodedFrameData& other)
                : MaterializedLightField(DeviceType::GPU),
                  frames_(other.frames_)
        { }

        GPUDecodedFrameData(GPUDecodedFrameData &&other) noexcept
                : MaterializedLightField(DeviceType::GPU),
                  frames_{std::move(other.frames_)}
        { }

        inline std::vector<GPUFrameReference>& frames() noexcept { return frames_; }
        inline explicit operator std::vector<GPUFrameReference>&() noexcept { return frames_; }
        inline explicit operator MaterializedLightFieldReference() override { return MaterializedLightFieldReference::make<GPUDecodedFrameData>(*this); }

    private:
        std::vector<GPUFrameReference> frames_{};
    };

    template<typename BaseType>
    class InterpolatedData: public BaseType {
    public:
        InterpolatedData(const shared_reference<BaseType> &base,
                         const interpolation::InterpolatorReference interpolator)
            : BaseType(base),
              base_(base),
              interpolator_(interpolator)
        { }

        inline explicit operator MaterializedLightFieldReference() override { return MaterializedLightFieldReference::make<InterpolatedData<BaseType>>(*this); }

    private:
        const MaterializedLightFieldReference base_;
        const interpolation::InterpolatorReference interpolator_;
    };

    class InterpolatedGPUDecodedFrameData : public InterpolatedData<GPUDecodedFrameData> {
    public:
        using InterpolatedData<GPUDecodedFrameData>::InterpolatedData;
    };

} // namespace lightdb::physical

#endif //LIGHTDB_DATA_H
