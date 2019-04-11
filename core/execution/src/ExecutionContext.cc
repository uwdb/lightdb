#include "ExecutionContext.h"
#include "PhysicalOperators.h"

namespace lightdb::execution {
    ExecutionContextReference NullExecutionContext::instance_ = ExecutionContextReference::make<NullExecutionContext>();

    void set_context(optimization::Plan &plan, const ExecutionContextReference &context) {
        std::for_each(plan.physical().begin(), plan.physical().end(),
                      [&context](auto &physical) {
                          physical->context(ExecutionContextReference(context)); });
    }

    ExecutionContext::ExecutionContext(optimization::Plan plan, const transactions::TransactionReference &transaction)
        : plan_(std::move(plan)),
          transaction_(transaction) {
        auto weak = this->weak_from_this();
        set_context(plan_, !weak.expired() ? ExecutionContextReference(weak.lock()) : *this);
    }

    ExecutionContext::~ExecutionContext() {
        set_context(plan_, NullExecutionContext::instance());
    }
} // namespace lightdb::execution