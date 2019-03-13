#include "PhysicalOperators.h"

namespace lightdb::runtime {
    Runtime<>::iterator Runtime<>::iterator::eos_instance_{};

    Runtime<>::Runtime(PhysicalLightField &physical)
            : physical_(physical),
              iterators_(functional::transform<Runtime<>::iterator>(
                          physical.parents().begin(), physical.parents().end(),
                          [](auto &parent) {
                              return parent->runtime()->begin(); }))
    { }

    LightFieldReference Runtime<>::logical() const {
        return physical().logical();
    }

} //namespace lightdb::runtime
