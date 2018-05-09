#ifndef LIGHTDB_COORDINATOR_H
#define LIGHTDB_COORDINATOR_H

#include "Plan.h"

namespace lightdb::execution {

class Coordinator {
public:
    PhysicalLightField& submit(const optimization::Plan &plan)
    {
        if(plan.sinks().size() != 1)
            throw NotImplementedError("Executing with more than one sink not yet supported.");
        else if(plan.assignments(plan.sinks()[0]).size() != 1)
            throw NotImplementedError("Executing with more than one sink assignment not yet supported.");

        const auto &sinks = plan.sinks();
        const auto &assignments = plan.assignments(sinks[0]);
        return *assignments[0];
    }

    void save(const optimization::Plan &plan, const std::string &filename)
    {
        std::ofstream file(filename);
        auto &output = submit(plan);

        for(const auto &data: output)
        {
            const auto &encoded = data.downcast<physical::CPUEncodedFrameData>();
            file.write((const char*)encoded.data(), encoded.size());
        }
    }
};

} // namespace lightdb::execution

#endif //LIGHTDB_COORDINATOR_H
