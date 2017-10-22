#include "Execution.h"
#include "Display.h"

namespace visualcloud::pipeline {

    template<typename ColorSpace>
    EncodedLightField execute(const LightFieldReference<ColorSpace> lightfield) {
        print_plan(lightfield);

        return EncodedLightField{"resources/red10.h264"};
    }

    template EncodedLightField execute<YUVColorSpace>(const LightFieldReference<YUVColorSpace>);
} // namespace visualcloud::pipeline
