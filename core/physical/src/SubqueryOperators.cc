#include "SubqueryOperators.h"
#include "HeuristicOptimizer.h"
#include "Display.h"

namespace lightdb::physical {

LightFieldReference GPUAngularSubquery::Runtime::ExecuteSubquery(const size_t index) {
    auto stream = streams()[index];
    auto selected = stream.Select(logical()->volume().components()[index]);
    return physical().subquery()(selected);
}

optimization::Plan GPUAngularSubquery::Runtime::CreatePlan() {
    auto unions = ExecuteSubquery(0);

    for(auto index = 1u; index < streams().size(); index++)
        unions = unions.Union(ExecuteSubquery(index));

    auto plan = physical().optimizer()->optimize(unions);

    LOG(INFO) << "Generated logical subquery:\n" << to_string(unions);
    LOG(INFO) << "Generated physical subquery:\n" << to_string(plan);

    return plan;
}

}; // namespace lightdb::physical
