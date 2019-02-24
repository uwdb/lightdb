#ifndef LIGHTDB_DISPLAY_H
#define LIGHTDB_DISPLAY_H

#include "Plan.h"

namespace lightdb {

void print_plan(const LightFieldReference&, const std::optional<optimization::Plan>& = {});
void print_plan(const optimization::Plan&);

}

#endif //LIGHTDB_DISPLAY_H
