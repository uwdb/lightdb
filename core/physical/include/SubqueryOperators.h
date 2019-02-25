#ifndef LIGHTDB_SUBQUERYOPERATORS_H
#define LIGHTDB_SUBQUERYOPERATORS_H

#include "Model.h"
#include "MaterializedLightField.h"
#include "PhysicalOperators.h"
#include "Adapters.h"
#include "Coordinator.h"
#include "Optimizer.h"

namespace lightdb::physical {

class GPUAngularSubquery: public GPUUnaryOperator<GPUDecodedFrameData> {
public:
    GPUAngularSubquery(const LightFieldReference &logical,
                       PhysicalLightFieldReference &parent,
                       const optimization::OptimizerReference &optimizer)
            : GPUUnaryOperator(logical, parent),
              optimizer_(optimizer),
              subquery_(logical.downcast<logical::SubqueriedLightField>().subquery()),
              type_(subquery_(LightFieldReference::make<logical::ConstantLightField>(YUVColor::red(), Point6D::zero()))),
              streams_([parent]() { return TeedPhysicalLightFieldAdapter::make(
                      parent, parent->logical()->volume().components().size()); }),
              subplan_([this]() { return CreatePlan(); }),
              subiterator_([this]() { return ExecutePlan(); })
    {
        if(logical->volume().components().size() <= 1)
            LOG(ERROR) << "Invalid angular subquery on " << logical->volume().components().size() << " components.";
        else if(logical->volume().components().size() > 64)
            LOG(WARNING) << "Angular subquery on " << logical->volume().components().size() << " components.";
    }

    std::optional<MaterializedLightFieldReference> read() override {
        if(*subiterator_ != subiterator_->eos())
            return (*subiterator_)++;
        else
            return {};
    }

    const LightField& subqueryType() const { return *type_; }

private:
    TeedPhysicalLightFieldAdapter& streams() { return *streams_.value(); }

    optimization::Plan CreatePlan();
    inline LightFieldReference ExecuteSubquery(size_t index);
    inline PhysicalLightField::iterator ExecutePlan() {
        LOG(WARNING) << "Arbitrarily creating new coordinator for subquery execution";
        return execution::Coordinator().submit<0u>(subplan_).begin();
    }

    const optimization::OptimizerReference optimizer_;
    const std::function<LightFieldReference(LightFieldReference)> subquery_;
    const LightFieldReference type_;
    lazy<std::shared_ptr<TeedPhysicalLightFieldAdapter>> streams_;
    lazy<optimization::Plan> subplan_;
    lazy<PhysicalLightField::iterator> subiterator_;
};

}; // namespace lightdb::physical

#endif //LIGHTDB_SUBQUERYOPERATORS_H
