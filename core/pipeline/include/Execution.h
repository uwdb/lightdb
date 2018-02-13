#ifndef LIGHTDB_EXECUTION_H
#define LIGHTDB_EXECUTION_H

#include "LightField.h"
#include "Encoding.h"

namespace lightdb::pipeline {
    lightdb::EncodedLightField execute(LightFieldReference, const std::string&);
}

#endif //LIGHTDB_EXECUTION_H
