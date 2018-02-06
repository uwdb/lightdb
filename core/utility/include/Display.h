#ifndef LIGHTDB_DISPLAY_H
#define LIGHTDB_DISPLAY_H

#include "LightField.h"
#include <queue>

namespace lightdb {

template<typename ColorSpace>
void print_plan(const LightFieldReference<ColorSpace> lightField) {
    std::deque<std::pair<LightFieldReference<ColorSpace>, int>> queue{{{lightField, 0u}}};

    while(!queue.empty()) {
        auto current = queue.front();
        queue.pop_front();

        printf("%*s\n",
               current.second + static_cast<int>(strlen(typeid(*current.first).name())),
               typeid(*current.first).name());

        for(const auto &field: current.first->provenance())
            queue.push_front({field, current.second + 4});
    }
}

}

#endif //LIGHTDB_DISPLAY_H
