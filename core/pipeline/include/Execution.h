#ifndef LIGHTDB_EXECUTION_H
#define LIGHTDB_EXECUTION_H

#include "LightField.h"
#include "Encoding.h"

namespace lightdb::pipeline {
    template<typename ColorSpace>
    lightdb::EncodedLightField execute(LightFieldReference<ColorSpace>, const std::string&);
}

#endif //LIGHTDB_EXECUTION_H
