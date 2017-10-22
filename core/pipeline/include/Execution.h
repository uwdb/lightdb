#ifndef VISUALCLOUD_EXECUTION_H_H
#define VISUALCLOUD_EXECUTION_H_H

#include "LightField.h"
#include "Encoding.h"

namespace visualcloud::pipeline {
    template<typename ColorSpace>
    EncodedLightField execute(LightFieldReference<ColorSpace>);
}

#endif //VISUALCLOUD_EXECUTION_H_H
