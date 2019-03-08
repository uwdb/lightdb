#ifndef LIGHTDB_DATA_H
#define LIGHTDB_DATA_H

#include "Encoding.h"
#include "DecodeReader.h"
#include "Pool.h"
#include "Frame.h"
#include "Codec.h"
#include <utility>

namespace lightdb::physical {
    class MaterializedLightField;
    using MaterializedLightFieldReference = shared_reference<MaterializedLightField, pool::BufferPoolEntry<MaterializedLightField>>;

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

        inline explicit operator MaterializedLightFieldReference() const { return ref(); }
        inline void accept(LightFieldVisitor& visitor) override { visitor.visit(*this); }

        virtual MaterializedLightFieldReference ref() const = 0;

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
        virtual const bytestring& value() = 0;

    protected:
        SerializableData(DeviceType device)
                : MaterializedLightField(device)
        { }
    };

    class SerializedData: public SerializableData {
    public:
        virtual const bytestring& value() { return *value_; }

    protected:
        SerializedData(DeviceType device, const bytestring &value)
            : SerializedData(device, value.begin(), value.end())
        { }

        template<typename Input>
        SerializedData(DeviceType device, Input begin, Input end)
                : SerializableData(device), value_(std::make_unique<bytestring>(begin, end))
        { }

    private:
        const std::shared_ptr<bytestring> value_;
    };

    class EmptyData: public SerializedData {
    public:
        EmptyData(DeviceType device) : SerializedData(device, {}) { }
        inline MaterializedLightFieldReference ref() const override { return MaterializedLightFieldReference::make<EmptyData>(*this); }
    };

    class FrameData: public SerializedData {
    public:
        const Configuration& configuration() const { return configuration_; }

    protected:
        FrameData(const DeviceType device, Configuration configuration)
                : SerializedData(device, {}),
                  configuration_(std::move(configuration))
        { }

        FrameData(const DeviceType device, Configuration configuration, const bytestring &value)
            : SerializedData(device, value),
              configuration_(std::move(configuration))
        { }

        template<typename Input>
        FrameData(const DeviceType device, Configuration configuration, Input &begin, const Input &end)
                : SerializedData(device, begin, end),
                  configuration_(std::move(configuration))
        { }

    private:
        const Configuration configuration_;
    };

    class EncodedFrameData: public FrameData {
    protected:
        EncodedFrameData(const DeviceType device, const Codec &codec,
                         const Configuration& configuration, const bytestring &value)
                : EncodedFrameData(device, codec, configuration, value.begin(), value.end())
        { }

        template<typename Input>
        EncodedFrameData(const DeviceType device, Codec codec, const Configuration& configuration,
                         Input begin, const Input end)
                : FrameData(device, configuration, begin, end), codec_(std::move(codec))
        { }

    public:
        const Codec& codec() const { return codec_; }

    private:
        const Codec codec_;
    };

    class CPUEncodedFrameData: public EncodedFrameData {
    public:
        explicit CPUEncodedFrameData(const Codec &codec, const Configuration& configuration, const bytestring &data)
                : EncodedFrameData(DeviceType::CPU, codec, configuration, data.begin(), data.end()),
                  packet_(data)
        { }

        explicit CPUEncodedFrameData(const Codec &codec, const Configuration& configuration,
                                     const DecodeReaderPacket &packet)
                : EncodedFrameData(DeviceType::CPU, codec, configuration,
                                   packet.payload, packet.payload + packet.payload_size),
                  packet_(packet)
        { }

        inline explicit operator const DecodeReaderPacket() const noexcept { return packet_; }
        inline MaterializedLightFieldReference ref() const override { return MaterializedLightFieldReference::make<CPUEncodedFrameData>(*this); }

    private:
        const DecodeReaderPacket packet_;
    };

    class CPUDecodedFrameData: public FrameData {
    public:
        CPUDecodedFrameData(const Configuration &configuration)
                : CPUDecodedFrameData(configuration, std::vector<LocalFrameReference>{})
        { }

        explicit CPUDecodedFrameData(const Configuration &configuration,
                                     std::vector<LocalFrameReference> frames)
                : FrameData(DeviceType::CPU, configuration),
                  frames_(std::move(frames))
        { }

        CPUDecodedFrameData(const CPUDecodedFrameData& other)
                : CPUDecodedFrameData(other.configuration(), other.frames_)
        { }

        CPUDecodedFrameData(CPUDecodedFrameData &&other) noexcept
                : CPUDecodedFrameData(other.configuration(), std::move(other.frames_))
        { }

        const bytestring& value() {
            if(serialized_.empty())
                std::for_each(frames().begin(), frames().end(),
                              [this](const auto &f) { serialized_.insert(serialized_.end(),
                                      f->data().begin(), f->data().end()); });
            return serialized_;
        }


        inline std::vector<LocalFrameReference>& frames() noexcept { return frames_; }
        inline const std::vector<LocalFrameReference>& frames() const noexcept { return frames_; }
        inline explicit operator std::vector<LocalFrameReference>&() noexcept { return frames_; }
        inline MaterializedLightFieldReference ref() const override { return MaterializedLightFieldReference::make<CPUDecodedFrameData>(*this); }

    private:
        std::vector<LocalFrameReference> frames_{};
        bytestring serialized_;
    };

    class GPUDecodedFrameData: public FrameData {
    public:
        GPUDecodedFrameData(const Configuration &configuration)
                : FrameData(DeviceType::GPU, configuration),
                  frames_{},
                  serialized_{}
        { }

        explicit GPUDecodedFrameData(const Configuration &configuration, std::vector<GPUFrameReference> frames)
                : FrameData(DeviceType::GPU, configuration),
                  frames_(std::move(frames)),
                  serialized_{}
        { }

        GPUDecodedFrameData(const GPUDecodedFrameData& other)
                : FrameData(DeviceType::GPU, other.configuration()),
                  frames_(other.frames_),
                  serialized_{}
        { }

        GPUDecodedFrameData(GPUDecodedFrameData &&other) noexcept
                : FrameData(DeviceType::GPU, other.configuration()),
                  frames_{std::move(other.frames_)},
                  serialized_{}
        { }

        inline std::vector<GPUFrameReference>& frames() noexcept { return frames_; }
        inline const std::vector<GPUFrameReference>& frames() const noexcept { return frames_; }
        inline explicit operator std::vector<GPUFrameReference>&() noexcept { return frames_; }
        inline MaterializedLightFieldReference ref() const override { return MaterializedLightFieldReference::make<GPUDecodedFrameData>(*this); }

        const bytestring& value() override {
            if(serialized_.empty())
                std::for_each(frames().begin(), frames().end(),
                              [this](const auto &gpu_frame) {
                                  LocalFrame cpu_frame{*gpu_frame->cuda()};
                                  serialized_.insert(serialized_.end(),
                                                     cpu_frame.data().begin(), cpu_frame.data().end()); });
            return serialized_;
        }

    private:
        std::vector<GPUFrameReference> frames_{};
        bytestring serialized_;
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

        inline MaterializedLightFieldReference ref() const override { return MaterializedLightFieldReference::make<InterpolatedData<BaseType>>(*this); }

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
