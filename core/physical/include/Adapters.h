#ifndef LIGHTDB_ADAPTERS_H
#define LIGHTDB_ADAPTERS_H

#include "LightField.h"
#include "MaterializedLightField.h"
#include <queue>
#include <mutex>

namespace lightdb::physical {

class PhysicalToLogicalLightFieldAdapter: public LightField {
public:
    explicit PhysicalToLogicalLightFieldAdapter(const PhysicalLightFieldReference &physical)
            : LightField({},
                         physical->logical()->volume(),
                         physical->logical()->colorSpace()),
              physical_(physical)
    { }

    PhysicalToLogicalLightFieldAdapter(PhysicalToLogicalLightFieldAdapter &) = default;
    PhysicalToLogicalLightFieldAdapter(const PhysicalToLogicalLightFieldAdapter &) = default;
    PhysicalToLogicalLightFieldAdapter(PhysicalToLogicalLightFieldAdapter &&) = default;

    ~PhysicalToLogicalLightFieldAdapter() override = default;

    void accept(LightFieldVisitor& visitor) override { visitor.visit(*this); }

    PhysicalLightFieldReference source() { return physical_; }

private:
    PhysicalLightFieldReference physical_;
};

class MaterializedToPhysicalOperatorAdapter: public PhysicalLightField {
public:
    explicit MaterializedToPhysicalOperatorAdapter(const LightFieldReference &logical,
                                                   const MaterializedLightFieldReference &source)
            : MaterializedToPhysicalOperatorAdapter(logical, source, {})
    { }

    explicit MaterializedToPhysicalOperatorAdapter(const LightFieldReference &logical,
                                                   const MaterializedLightFieldReference &source,
                                                   const PhysicalLightFieldReference &parent)
            : MaterializedToPhysicalOperatorAdapter(logical, source, std::vector<PhysicalLightFieldReference>{parent})
    { }

    explicit MaterializedToPhysicalOperatorAdapter(const LightFieldReference &logical,
                                                   const MaterializedLightFieldReference &source,
                                                   const std::vector<PhysicalLightFieldReference> &parents)
            : PhysicalLightField(logical,
                                 parents,
                                 source->device(),
                                 runtime::make<Runtime>(*this, source))
    { }

    MaterializedToPhysicalOperatorAdapter(const MaterializedToPhysicalOperatorAdapter &) = default;
    MaterializedToPhysicalOperatorAdapter(MaterializedToPhysicalOperatorAdapter &&) = default;

    ~MaterializedToPhysicalOperatorAdapter() override = default;

private:
    class Runtime: public runtime::Runtime<> {
    public:
        explicit Runtime(PhysicalLightField &physical, const MaterializedLightFieldReference &source)
            : runtime::Runtime<>(physical),
              source_(source),
              read_(false)
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(!read_) {
                read_ = false;
                return {source_};
            } else
                return {};
        }

    private:
        MaterializedLightFieldReference source_;
        bool read_;
    };

};

class TeedPhysicalLightFieldAdapter {
public:
    static std::shared_ptr<TeedPhysicalLightFieldAdapter> make(const PhysicalLightFieldReference &source,
                                                               const size_t size) {
        return std::make_shared<TeedPhysicalLightFieldAdapter>(source, size);
    }

    TeedPhysicalLightFieldAdapter(const PhysicalLightFieldReference &source,
                                  const size_t size) {
        auto mutex = std::make_shared<std::mutex>();
        auto queues = std::make_shared<std::vector<std::queue<MaterializedLightFieldReference>>>(size);

        for(auto index = 0u; index < size; index++)
            if(source.is<GPUOperator>())
                tees_.emplace_back(
                        LightFieldReference::make<PhysicalToLogicalLightFieldAdapter>(
                                    PhysicalLightFieldReference::make<GPUTeedPhysicalLightField>(
                                            mutex, queues, source, index, source.downcast<GPUOperator>().gpu())));
            else
                tees_.emplace_back(
                        LightFieldReference::make<PhysicalToLogicalLightFieldAdapter>(
                                    PhysicalLightFieldReference::make<TeedPhysicalLightField>(
                                            mutex, queues, source, index)));
    }

    TeedPhysicalLightFieldAdapter(const TeedPhysicalLightFieldAdapter&) = delete;
    TeedPhysicalLightFieldAdapter(TeedPhysicalLightFieldAdapter&& other) = default;

    size_t size() const {
        return tees_.size();
    }

    LightFieldReference operator[] (const size_t index) {
        return tees_.at(index);
    }

    PhysicalLightFieldReference physical(const size_t index) {
        return (*this)[index].downcast<PhysicalToLogicalLightFieldAdapter>().source();
    }

    //TODO these should be private
    class TeedPhysicalLightField: public PhysicalLightField {
    public:
        TeedPhysicalLightField(std::shared_ptr<std::mutex> mutex,
                               std::shared_ptr<std::vector<std::queue<MaterializedLightFieldReference>>> queues,
                               const PhysicalLightFieldReference &source,
                               const size_t index)
                : PhysicalLightField(source->logical(),
                                     {source},
                                     source->device(),
                                     runtime::make<Runtime>(*this, mutex, queues, source, index))
        { }

        TeedPhysicalLightField(const TeedPhysicalLightField&) = delete;
        TeedPhysicalLightField(TeedPhysicalLightField&&) noexcept = default;

    protected:
        class Runtime: public runtime::Runtime<> {
        public:
            explicit Runtime(PhysicalLightField &physical,
                             std::shared_ptr<std::mutex> mutex,
                             std::shared_ptr<std::vector<std::queue<MaterializedLightFieldReference>>> queues,
                             const PhysicalLightFieldReference &source,
                             const size_t index)
                 : runtime::Runtime<>(physical),
                   index_(index),
                   mutex_(std::move(mutex)),
                   source_(source),
                   queues_(std::move(queues))
            { }

            std::optional<physical::MaterializedLightFieldReference> read() override {
                std::optional<physical::MaterializedLightFieldReference> value;

                std::lock_guard lock(*mutex_);

                auto &queue = queues_->at(index_);

                if(queue.empty())
                    // Teed stream's queue is empty, so read from base stream and broadcast to all queues
                    if((value = functional::single(iterators())++).has_value())
                        std::for_each(queues_->begin(), queues_->end(), [&value](auto &q) { q.push(value.value()); });

                // Now return a value (if any) from this tee's queue
                if(!queue.empty()) {
                    value = queue.front();
                    queue.pop();
                    return value;
                } else
                    return {};
            }

            const size_t index_;
            std::shared_ptr<std::mutex> mutex_;
            PhysicalLightFieldReference source_;
            std::shared_ptr<std::vector<std::queue<MaterializedLightFieldReference>>> queues_;
        };
    };

    class GPUTeedPhysicalLightField : public TeedPhysicalLightField, public GPUOperator {
    public:
        GPUTeedPhysicalLightField(std::shared_ptr<std::mutex> mutex,
                                  std::shared_ptr<std::vector<std::queue<MaterializedLightFieldReference>>> queues,
                                  const PhysicalLightFieldReference &source,
                                  const size_t index,
                                  const execution::GPU& gpu)
                : TeedPhysicalLightField(std::move(mutex), std::move(queues), source, index),
                  GPUOperator(gpu)
        { }
    };

private:
    std::vector<LightFieldReference> tees_;
};

}; //namespace lightdb::physical

#endif //LIGHTDB_ADAPTERS_H
