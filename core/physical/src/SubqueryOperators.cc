#include "SubqueryOperators.h"
#include "HeuristicOptimizer.h"

namespace lightdb::physical {

LightFieldReference GPUAngularSubquery::ExecuteSubquery(const size_t index) {
    auto stream = streams()[index];
    auto selected = stream.Select(logical()->volume().components()[index]);
    return subquery_(selected);
}

optimization::Plan GPUAngularSubquery::CreatePlan() {
    auto unions = ExecuteSubquery(0);

    for(auto index = 1u; index < streams().size(); index++)
        unions = unions.Union(ExecuteSubquery(index));

    return optimizer_->optimize(unions);
}

}; // namespace lightdb::physical
