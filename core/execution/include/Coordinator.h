#ifndef LIGHTDB_COORDINATOR_H
#define LIGHTDB_COORDINATOR_H

#include "Plan.h"
#include "Pool.h"
#include "progress.h"

namespace lightdb::execution {

class Coordinator {
public:
    template<unsigned int Index>
    PhysicalLightField& submit(const optimization::Plan &plan) {
        auto assignments = submit(plan);

        CHECK_GE(assignments.size(), Index);

        return *assignments[Index];
    }

    std::vector<PhysicalLightFieldReference> submit(const optimization::Plan &plan) {
        const auto &sinks = plan.sinks();
        return functional::flatmap<std::vector<PhysicalLightFieldReference>>(sinks.begin(), sinks.end(), [plan](auto &sink) { return plan.unassigned(sink); });
    }

    void save(const optimization::Plan &plan, const std::string &filename) {
        save(plan, std::vector<std::string>{filename});
    }

    //TODO this should be in the algebra, returning an external TLF
    void save(const optimization::Plan &plan, const std::vector<std::string> &filenames) {
        auto streams = functional::transform<std::ofstream>(filenames.begin(), filenames.end(), [](auto &filename) { return std::ofstream(filename); });
        save(plan, functional::transform<std::ostream*>(streams.begin(), streams.end(), [](auto &s) { return &s; }));
    }

    void save(const optimization::Plan &plan, std::ostream& stream) {
        save(plan, std::vector<std::ostream*>{&stream});
    }

    std::string save(const optimization::Plan &plan) {
        std::ostringstream stream;
        save(plan, std::vector<std::ostream*>{&stream});
        return stream.str();
    }

    void save(const optimization::Plan &plan, std::vector<std::ostream*> streams) {
        auto outputs = submit(plan);
        auto iterators = functional::transform<PhysicalLightField::iterator>(outputs.begin(), outputs.end(), [](auto &out) { return out->begin(); });
        Progress progress(static_cast<int>(iterators.size()));

        CHECK_EQ(iterators.size(), streams.size());

        while(!iterators.empty()) {
            for(auto index = 0u; index < iterators.size(); index++) {
                if(iterators[index] != iterators[index].eos()) {
                    auto next = iterators[index]++;
                    auto &data = next.downcast<physical::SerializableData>();
                    std::copy(data.value().begin(), data.value().end(),
                              std::ostreambuf_iterator<char>(*streams[index]));
                } else {
                    iterators.erase(iterators.begin() + index);
                    streams.erase(streams.begin() + index);
                    index--;
                    progress++;
                }
            }
        }
    }
};

} // namespace lightdb::execution

#endif //LIGHTDB_COORDINATOR_H
