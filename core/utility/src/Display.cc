#include "Display.h"
#include "PhysicalOperators.h"
#include <queue>

namespace lightdb {
    #ifdef __GNUG__
        #include <cxxabi.h>

        std::string demangle(const char* name) {
            int status;

            std::unique_ptr<char, void(*)(void*)> res {
                    abi::__cxa_demangle(name, nullptr, nullptr, &status),
                    std::free
            };

            return status == 0 ? res.get() : name;
        }
    #else
        // Identity transform if not g++
        std::string demangle(const char* name) { return name; }
    #endif

    std::string to_string(const std::vector<LightFieldReference> &fields) {
        std::string result;

        for(const auto &field: fields)
            result += to_string(field);

        return result;
    }

    std::string to_string(const LightFieldReference &lightField) {
        std::deque<std::pair<LightFieldReference, unsigned int>> queue{{{lightField, 0u}}};
        std::string result;

        while(!queue.empty()) {
            const auto current = queue.front();
            const auto &ref = *current.first;
            const auto name = demangle(typeid(ref).name());
            queue.pop_front();

            result += std::string(current.second, ' ') + name + '\n';

            for(const auto &field: current.first->parents())
                queue.push_front({field, current.second + 2});
        }

        return result;
    }

    std::string to_string(const optimization::Plan &plan) {
        std::deque<std::pair<PhysicalOperatorReference, unsigned int>> queue{};
        std::string result;

        for(auto &physical: plan.physical())
            if (plan.children(physical).empty())
                queue.push_front({physical, 0u});

        while(!queue.empty()) {
            const auto current = queue.front();
            const auto &ref = *current.first;
            const auto name = demangle(typeid(ref).name());
            queue.pop_front();

            result += std::string(current.second, ' ') + name + '\n';

            for(const auto &field: current.first->parents())
                queue.push_front({field, current.second + 2});
        }

        return result;
    }

} // namespace lightdb