#ifndef PYLIGHTDB_ENVIRONMENT_H
#define PYLIGHTDB_ENVIRONMENT_H

#include "GPUContext.h"

namespace lightdb::python {

class GPU {
    public:
        explicit GPU(unsigned int device) : device_(device) { }

        unsigned int device() const { return device_; }
        GPUContext context() const { return GPUContext(device()); }

    private:
        unsigned int device_;
};

class PythonEnvironment {
    public:
        const std::vector<GPU>& gpus() const { return gpus_; }

    protected:
        explicit PythonEnvironment(std::vector<GPU> gpus)
                : gpus_(std::move(gpus))
        { }

    private:
        const std::vector<GPU> gpus_;
};

class LocalEnvironment: public PythonEnvironment {
    public:
        explicit LocalEnvironment(bool include_gpus=true)
                : PythonEnvironment(include_gpus ? GetLocalGPUs() : std::vector<GPU>{})
        { }

    private:
        static std::vector<GPU> GetLocalGPUs() {
            std::vector<GPU> gpus;
            size_t count = GPUContext::device_count();

            for(auto index = 0u; index < count; index++)
                gpus.emplace_back(GPU{index});
            return gpus;
        }
};

}



#endif // PYLIGHTDB_ENVIRONMENT_H 