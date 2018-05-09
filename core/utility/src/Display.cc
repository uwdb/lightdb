#include "Display.h"
#include <queue>

namespace lightdb {

    void print_plan(const LightFieldReference &lightField) {
        std::deque<std::pair<LightFieldReference, unsigned int>> queue{{{lightField, 0u}}};

        while(!queue.empty()) {
            auto current = queue.front();
            queue.pop_front();

            printf("%*s\n",
                   current.second + static_cast<int>(strlen(typeid(*current.first).name())),
                   typeid(*current.first).name());

            for(const auto &field: current.first->parents())
                queue.push_front({field, current.second + 4});
        }
    }

    void print_plan(optimization::Plan &plan) {
        std::deque<std::pair<PhysicalLightFieldReference, unsigned int>> queue{};

        for(auto &sink: plan.sinks()) {
            print_plan(sink);
            for(auto &physical: plan.assignments(sink))
                queue.push_front({physical, 0u});
            }

        while(!queue.empty()) {
            auto current = queue.front();
            queue.pop_front();

            printf("%*s\n",
                   current.second + static_cast<int>(strlen(typeid(*current.first).name())),
                   typeid(*current.first).name());

            for(const auto &field: current.first->parents())
                queue.push_front({field, current.second + 4});
        }
    }

} // namespace lightdb