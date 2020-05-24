#ifndef LIGHTDB_ESTIMATOR_H
#define LIGHTDB_ESTIMATOR_H

#include "Environment.h"
#include "reference.h"

namespace lightdb::optimization {

class Allocator {
public:
    virtual const execution::GPU gpu() = 0;
    virtual const execution::GPU gpu(unsigned int cost) = 0;

protected:
    explicit Allocator(execution::Environment environment)
            : environment_(std::move(environment))
    { }

    const execution::Environment &environment() const { return environment_; }

private:
    const execution::Environment environment_;
};

using AllocatorReference = shared_reference<Allocator>;

class RoundRobinAllocator : public Allocator {
public:
    explicit RoundRobinAllocator(const execution::Environment &environment)
        : Allocator(environment),
          last_gpu_(0u)
    { }

    const execution::GPU gpu() override { return gpu(0u); }

    const execution::GPU gpu(const unsigned int cost) override {
        auto &gpu = environment().gpus().at(last_gpu_++ % environment().gpus().size());
        LOG(INFO) << "Assigning GPU " << gpu.device();
        return gpu;
    }


private:
    size_t last_gpu_;
};

} // namespace lightdb::optimization

#endif //LIGHTDB_ESTIMATOR_H
