#ifndef LIGHTDB_DISPLAY_H
#define LIGHTDB_DISPLAY_H

#include "Plan.h"

namespace lightdb {

void print_plan(const LightFieldReference &lightField);
void print_plan(const optimization::Plan &plan);

}

#endif //LIGHTDB_DISPLAY_H
