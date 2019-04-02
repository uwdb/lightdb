#include "ExecutionContext.h"
#include "PhysicalOperators.h"

namespace lightdb::execution {
    ExecutionContextReference NullExecutionContext::instance_ = ExecutionContextReference::make<NullExecutionContext>();

    ExecutionContext::ExecutionContext(optimization::Plan plan, const transactions::TransactionReference &transaction)
        : plan_(std::move(plan)),
          transaction_(transaction) {
        std::for_each(plan_.physical().begin(), plan_.physical().end(),
                      [this](auto &physical) { physical->context(*this); });
    }

} // namespace lightdb::execution