#ifndef LIGHTDB_COORDINATOR_H
#define LIGHTDB_COORDINATOR_H

#include "Plan.h"

namespace lightdb::execution {

class Coordinator {
public:
    template<unsigned int Index>
    PhysicalLightField& submit(const optimization::Plan &plan) {
        auto assignments = submit(plan);

        CHECK_GE(assignments.size(), Index);

        return *assignments[Index];
    }

    std::vector<PhysicalLightFieldReference> submit(const optimization::Plan &plan)
    {
        const auto &sinks = plan.sinks();
        return functional::flatmap<std::vector<PhysicalLightFieldReference>>(sinks.begin(), sinks.end(), [plan](auto &sink) { return plan.unassigned(sink); });
    }

    void save(const optimization::Plan &plan, const std::string &filename)
    {
        save(plan, std::vector<std::string>{filename});
    }

    //TODO this should be in the algebra, returning an external TLF
    void save(const optimization::Plan &plan, const std::vector<std::string> &filenames)
    {
        auto outputs = submit(plan);
        auto iterators = functional::transform<PhysicalLightField::iterator>(outputs.begin(), outputs.end(), [](auto &out) { return out->begin(); });
        auto streams = functional::transform<std::ofstream>(filenames.begin(), filenames.end(), [](auto &filename) { return std::ofstream(filename); });

        CHECK_EQ(iterators.size(), streams.size());

        while(!iterators.empty()) {
            for(auto index = 0u; index < iterators.size(); index++) {
                if(iterators[index] != iterators[index].eos()) {
                    auto encoded = (iterators[index]++).downcast<physical::CPUEncodedFrameData>();
                    std::copy(encoded.value().begin(), encoded.value().end(),
                              std::ostreambuf_iterator<char>(streams[index]));
                } else {
                    iterators.erase(iterators.begin() + index);
                    streams.erase(streams.begin() + index);
                    index--;
                }
            }
        }
    }
};

} // namespace lightdb::execution

#endif //LIGHTDB_COORDINATOR_H
