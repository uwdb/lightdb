#ifndef LIGHTDB_ENVIRONMENT_H
#define LIGHTDB_ENVIRONMENT_H

#include "GPUContext.h"

namespace lightdb::execution {

class GPU {
public:
    explicit GPU(unsigned int device) : device_(device) { }

    unsigned int device() const { return device_; }
    GPUContext context() const { return GPUContext(device()); }

private:
    unsigned int device_;
};

class Environment {
public:
    const std::vector<GPU>& gpus() const { return gpus_; }

protected:
    explicit Environment(std::vector<GPU> gpus)
            : gpus_(std::move(gpus))
    { }

private:
    const std::vector<GPU> gpus_;
};

class LocalEnvironment: public Environment {
public:
    LocalEnvironment()
            : Environment({GPU(0)})
    {
        LOG(INFO) << "Local environment hardcoded with one stub GPU; should detect them.";
    }
};

} // namespace lightdb::execution

#endif //LIGHTDB_ENVIRONMENT_H
