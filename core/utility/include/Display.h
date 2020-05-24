#ifndef LIGHTDB_DISPLAY_H
#define LIGHTDB_DISPLAY_H

#include "Plan.h"

namespace lightdb {

std::string to_string(const std::vector<LightFieldReference>&);
std::string to_string(const LightFieldReference&);
std::string to_string(const optimization::Plan&);

}

#endif //LIGHTDB_DISPLAY_H
