#include "Display.h"
#include <queue>

namespace lightdb {

    void print_plan(const LightFieldReference &lightField, const std::optional<optimization::Plan> &plan={}) {
        std::deque<std::pair<LightFieldReference, unsigned int>> queue{{{lightField, 0u}}};

        while(!queue.empty()) {
            auto current = queue.front();
            queue.pop_front();

            printf("L:%*s %s\n",
                   current.second + static_cast<int>(strlen(typeid(*current.first).name())),
                   typeid(*current.first).name(),
                   (plan.has_value() ? std::to_string(plan.value().assignments(current.first).size())
                                     : std::string{}).c_str());

            for(const auto &field: current.first->parents())
                queue.push_front({field, current.second + 4});
        }
    }

    void print_plan(const optimization::Plan &plan) {
        std::deque<std::pair<PhysicalLightFieldReference, unsigned int>> queue{};

        for(auto &sink: plan.sinks()) {
            print_plan(sink, plan);

            for(auto &physical: plan.assignments(sink))
                queue.push_front({physical, 0u});
            }

        while(!queue.empty()) {
            auto current = queue.front();
            queue.pop_front();

            printf("P:%*s\n",
                   current.second + static_cast<int>(strlen(typeid(*current.first).name())),
                   typeid(*current.first).name());

            for(const auto &field: current.first->parents())
                queue.push_front({field, current.second + 4});
        }
        printf("===\n");
    }

} // namespace lightdb