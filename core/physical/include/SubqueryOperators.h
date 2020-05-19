#ifndef LIGHTDB_SUBQUERYOPERATORS_H
#define LIGHTDB_SUBQUERYOPERATORS_H

#include "Model.h"
#include "MaterializedLightField.h"
#include "PhysicalOperators.h"
#include "Adapters.h"
#include "Coordinator.h"
#include "Optimizer.h"

namespace lightdb::physical {

class GPUAngularSubquery: public PhysicalOperator, public GPUOperator, UnaryOperator {
public:
    GPUAngularSubquery(const LightFieldReference &logical,
                       PhysicalOperatorReference &parent,
                       const optimization::OptimizerReference &optimizer)
            : PhysicalOperator(logical, {parent}, DeviceType::GPU, runtime::make<Runtime>(*this)),
              GPUOperator(parent),
              optimizer_(optimizer),
              subquery_(logical.downcast<logical::SubqueriedLightField>().subquery()),
              type_(subquery_(LightFieldReference::make<logical::ConstantLightField>(YUVColor::red(), Point6D::zero()))) {
        if(logical->volume().components().size() <= 1)
            LOG(ERROR) << "Invalid angular subquery on " << logical->volume().components().size() << " components.";
        else if(logical->volume().components().size() > 64)
            LOG(WARNING) << "Angular subquery on " << logical->volume().components().size() << " components.";
    }

    const std::function<LightFieldReference(LightFieldReference)>& subquery() const { return subquery_; }
    const LightField& subqueryType() const { return *type_; }
    optimization::OptimizerReference optimizer() const { return optimizer_; }

private:
    class Runtime: public runtime::UnaryRuntime<GPUAngularSubquery, GPUDecodedFrameData> {
    public:
        explicit Runtime(GPUAngularSubquery &physical)
            : runtime::UnaryRuntime<GPUAngularSubquery, GPUDecodedFrameData>(physical),
              streams_(TeedPhysicalOperatorAdapter::make(
                      physical.parent(),
                      logical()->volume().components().size())),
              subplan_(CreatePlan()),
              subiterator_(ExecutePlan())
        { }

        std::optional<physical::MaterializedLightFieldReference> read() override {
            if(subiterator_ != subiterator_.eos())
                return subiterator_++;
            else
                return {};
        }

    private:
        TeedPhysicalOperatorAdapter& streams() { return *streams_; }

        optimization::Plan CreatePlan();
        inline LightFieldReference ExecuteSubquery(size_t index);
        inline runtime::RuntimeIterator ExecutePlan() {
            LOG(WARNING) << "Arbitrarily creating new coordinator for subquery execution";
            return execution::Coordinator().submit<0u>(subplan_).runtime()->begin();
        }

        std::shared_ptr<TeedPhysicalOperatorAdapter> streams_;
        optimization::Plan subplan_;
        runtime::RuntimeIterator subiterator_;
    };

    const optimization::OptimizerReference optimizer_;
    const std::function<LightFieldReference(LightFieldReference)> subquery_;
    const LightFieldReference type_;
};

}; // namespace lightdb::physical

#endif //LIGHTDB_SUBQUERYOPERATORS_H
