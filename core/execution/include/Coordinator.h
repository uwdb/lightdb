#ifndef LIGHTDB_COORDINATOR_H
#define LIGHTDB_COORDINATOR_H

#include "Plan.h"
#include "Optimizer.h"
#include "Pool.h"
#include "progress.h"

namespace lightdb::execution {

class Coordinator {
public:
    template<unsigned int Index>
    PhysicalOperator& submit(const optimization::Plan &plan) {
        auto assignments = submit(plan);

        if(assignments.size() <= Index)
            throw CoordinatorError(std::string("Could not execute sink ") + std::to_string(Index) +
                                   "; only " + std::to_string(assignments.size()) + " were found");

        return *assignments[Index];
    }

    std::vector<PhysicalOperatorReference> submit(const optimization::Plan &plan) {
        const auto &sinks = plan.sinks();
        return functional::flatmap<std::vector<PhysicalOperatorReference>>(sinks.begin(), sinks.end(),
                [plan](auto &sink) { return plan.unassigned(sink); });
    }

    [[deprecated]]
    void save(const optimization::Plan &plan, const std::string &filename) {
        save(plan, std::vector<std::string>{filename});
    }

    [[deprecated]]
    void save(const optimization::Plan &plan, const std::vector<std::string> &filenames) {
        auto streams = functional::transform<std::ofstream>(filenames.begin(), filenames.end(),
                                                            [](auto &filename) { return std::ofstream(filename); });
        save(plan, functional::transform<std::ostream*>(streams.begin(), streams.end(), [](auto &s) { return &s; }));
    }

    void execute(const LightFieldReference &query) {
        execute(query, optimization::Optimizer::instance());
    }

    void execute(const LightFieldReference &query, const optimization::Optimizer& optimizer) {
        execute(std::vector<LightFieldReference>{query}, optimizer);
    }

    void execute(const std::vector<LightFieldReference> &query) {
        execute(query, optimization::Optimizer::instance());
    }

    void execute(const std::vector<LightFieldReference> &query, const optimization::Optimizer& optimizer) {
        execute(optimizer.optimize(query));
    }

    void execute(const optimization::Plan &plan) {
        auto outputs = submit(plan);
        auto iterators = functional::transform<runtime::RuntimeIterator>(
                outputs.begin(), outputs.end(),
                [](auto &out) { return out->runtime()->begin(); });
        Progress progress(static_cast<int>(iterators.size()));

        while(!iterators.empty()) {
            iterators.erase(std::remove_if(iterators.begin(), iterators.end(),
                                           [](auto &it) { ++it; return it == it.eos(); }),
                            iterators.end());
            progress.display(iterators.size());
        }
    }

    std::string serialize(const LightFieldReference &query) {
        return functional::single(serialize(optimization::Optimizer::instance().optimize(query)));
    }

    std::vector<std::string> serialize(const std::vector<LightFieldReference> &queries) {
        return serialize(optimization::Optimizer::instance().optimize(queries));
    }

    std::vector<std::string> serialize(const optimization::Plan &plan) {
        auto outputs = submit(plan);
        auto iterators = functional::transform<std::pair<runtime::RuntimeIterator, std::ostringstream>>(
                outputs.begin(), outputs.end(),
                [](auto &out) { return std::make_pair(out->runtime()->begin(), std::ostringstream{}); });
        std::vector<std::string> result;
        Progress progress(static_cast<int>(iterators.size()));

        while(!iterators.empty()) {
            iterators.erase(std::remove_if(iterators.begin(), iterators.end(),
                                          [&result](auto &pair) {
                auto &[it, stream] = pair;
                if(it != it.eos()) {
                    auto next = it++;
                    auto &data = next.template downcast<physical::SerializableData>();
                    std::copy(data.value().begin(), data.value().end(),
                              std::ostreambuf_iterator<char>(stream));
                    return false;
                } else {
                    result.push_back(stream.str());
                    return true;
                } }), iterators.end());
            progress.display(iterators.size());
        }

        return result;
    }

    std::string save(const optimization::Plan &plan) {
        std::ostringstream stream;
        save(plan, std::vector<std::ostream*>{&stream});
        return stream.str();
    }

    [[deprecated]]
    void save(const optimization::Plan &plan, std::vector<std::ostream*> streams) {
        auto outputs = submit(plan);
        auto iterators = functional::transform<runtime::RuntimeIterator>(outputs.begin(), outputs.end(), [](auto &out) { return out->runtime()->begin(); });
        Progress progress(static_cast<int>(iterators.size()));

        if(iterators.size() != streams.size())
            throw CoordinatorError("Plan does not contain a consistent physical plan.");

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
