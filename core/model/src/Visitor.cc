#include "LightField.h"
#include "Visitor.h"

namespace lightdb::internal {

void VisitorDelegator::accept_delegate(LightFieldVisitor &visitor, const LightFieldReference &field) {
    field->accept(visitor);
}

} // namespace lightdb::internal

