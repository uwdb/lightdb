#ifndef VISUALCLOUD_EXECUTION_H_H
#define VISUALCLOUD_EXECUTION_H_H

#include "LightField.h"
#include "Encoding.h"

namespace visualcloud::pipeline {
    template<typename ColorSpace>
    visualcloud::EncodedLightField execute(LightFieldReference<ColorSpace>, const std::string&);
}

#endif //VISUALCLOUD_EXECUTION_H_H
