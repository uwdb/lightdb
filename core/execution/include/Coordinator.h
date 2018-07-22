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
        else if(plan.assignments(plan.sinks()[0]).empty())
            throw NotImplementedError("Cannot execute with no assignment for sink");
        //else if(plan.assignments(plan.sinks()[0]).size() > 1)
         //   throw NotImplementedError("Executing with more than one sink assignment not yet supported.");

        const auto &sinks = plan.sinks();
        const auto &assignments = plan.assignments(sinks[0]);

        if(assignments.size() > 1)
            LOG(WARNING) << "Found more than one sink assignment; randomly selecting last one.";

        return *assignments[assignments.size() - 1];
    }

    //TODO this should be in the algebra, returning an external TLF
    void save(const optimization::Plan &plan, const std::string &filename)
    {
        std::ofstream file(filename);
        auto &output = submit(plan);

        for(const auto &data: output)
        {
            auto &encoded = data.downcast<physical::CPUEncodedFrameData>();
            std::copy(encoded.value().begin(),encoded.value().end(),std::ostreambuf_iterator<char>(file));
        }
    }
};

} // namespace lightdb::execution

#endif //LIGHTDB_COORDINATOR_H
