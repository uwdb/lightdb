#ifndef LIGHTDB_RULES_H
#define LIGHTDB_RULES_H

#include "Model.h"
#include "Optimizer.h"
#include "ScanOperators.h"
#include "EncodeOperators.h"
#include "DecodeOperators.h"
#include "UnionOperators.h"
#include "MapOperators.h"
#include "TransferOperators.h"
#include "DiscretizeOperators.h"
#include "InterpolateOperators.h"
#include "SubqueryOperators.h"
#include "IdentityOperators.h"
#include "HomomorphicOperators.h"
#include "SubsetOperators.h"
#include "StoreOperators.h"

namespace lightdb::optimization {
    class ChooseMaterializedScans : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const LightField &node) override {
            if (plan().has_physical_assignment(node))
                return false;
            else if(node.is<physical::GPUDecodedFrameData>()) {
                auto ref = plan().lookup(node);
                auto &m = node.downcast<physical::GPUDecodedFrameData>();
                auto mref = physical::MaterializedLightFieldReference::make<physical::GPUDecodedFrameData>(m);

                //Removed this line without actually testing the consequences
                //plan().emplace<physical::GPUScanMemory>(ref, mref);
                plan().emplace<physical::GPUOperatorAdapter>(mref);
                return true;
            } else if(node.is<physical::PhysicalToLogicalLightFieldAdapter>()) {
                auto ref = plan().lookup(node);
                auto op = plan().emplace<physical::GPUOperatorAdapter>(ref.downcast<physical::PhysicalToLogicalLightFieldAdapter>().source());
                plan().assign(ref, op);
                return true;
            } else
                return false;
        }
    };

    class ChooseDecoders : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::ScannedLightField &node) override {
            if(!plan().has_physical_assignment(node)) {
                LOG(WARNING) << "Just using first stream and GPU and ignoring all others";
                auto gpu = plan().environment().gpus()[0];
                auto stream = node.metadata().streams()[0];

                auto logical = plan().lookup(node);
                auto &scan = plan().emplace<physical::ScanSingleFile>(logical, stream);
                auto decode = plan().emplace<physical::GPUDecode>(logical, scan, gpu);
                return true;
            }
            return false;
        }
    };

    class ChooseEncoders : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::EncodedLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecode>()
                                   ? physical_parents[0]
                                   : physical_parents[physical_parents.size() - 1];

            if(!plan().has_physical_assignment(node)) {
                LOG(WARNING) << "Randomly picking HEVC as codec";
                if(hardcoded_parent.is<physical::GPUAngularSubquery>() && hardcoded_parent.downcast<physical::GPUAngularSubquery>().subqueryType().is<logical::EncodedLightField>())
                    plan().emplace<physical::CPUIdentity>(plan().lookup(node), hardcoded_parent);
                    //plan().emplace<physical::HomomorphicUniformTile>(plan().lookup(node), hardcoded_parent, 4, 4);
                else if(hardcoded_parent.is<physical::GPUOperator>())
                    plan().emplace<physical::GPUEncode>(plan().lookup(node), hardcoded_parent, Codec::hevc());
                //else if(hardcoded_parent.is<physical::GPUScanMemory>() && hardcoded_parent->device() == physical::DeviceType::GPU)
                //    plan().emplace<physical::GPUEncode>(plan().lookup(node), hardcoded_parent, Codec::hevc());
                else if(hardcoded_parent.is<physical::CPUMap>() && hardcoded_parent.downcast<physical::CPUMap>().transform()(physical::DeviceType::CPU).codec().name() == node.codec().name())
                    plan().emplace<physical::CPUIdentity>(plan().lookup(node), hardcoded_parent);
                else {
                    auto gpu = plan().environment().gpus()[0];
                    auto transfer = plan().emplace<physical::CPUtoGPUTransfer>(plan().lookup(node), hardcoded_parent, gpu);
                    plan().emplace<physical::GPUEncode>(plan().lookup(node), transfer, Codec::hevc());
                }
                return true;
            } else {

            }
            return false;
        }
    };

    class ChooseUnion : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        std::optional<PhysicalLightFieldReference> FindEncodeParent(const LightFieldReference &node) {
            for(const auto &parent: node->parents()) {
                if(parent.is<logical::EncodedLightField>() &&
                   plan().has_physical_assignment(parent))
                    return {plan().assignments(parent)[0]};
            }
            return {};
        }

        std::optional<PhysicalLightFieldReference> FindHomomorphicAncestor(const LightFieldReference &node) {
            std::deque<PhysicalLightFieldReference> queue;

            for(const auto &parent: node->parents())
                for(const auto &assignment: plan().assignments(parent))
                    queue.push_back(assignment);

            while(!queue.empty()) {
                auto element = queue.front();
                queue.pop_front();

                if(element->logical().is<logical::CompositeLightField>() &&
                   element.is<physical::HomomorphicUniformAngularUnion>())
                    return element;
                else if(element.is<physical::CPUIdentity>())
                    for(const auto &parent: element->parents())
                        queue.push_back(parent);
            }

            return {};
        }

        bool visit(const logical::CompositeLightField &node) override {
            if(plan().has_physical_assignment(node))
                return false;
            else if(node.parents().size() != 2)
                return false;
            else if(node.parents()[0].is<logical::EncodedLightField>() &&
                    node.parents()[1].is<logical::EncodedLightField>()) {
                std::vector<PhysicalLightFieldReference> physical_outputs;
                for (auto &parent: node.parents()) {
                    const auto &assignments = plan().assignments(parent);
                    if (assignments.empty())
                        return false;
                    physical_outputs.push_back(assignments[assignments.size() - 1]);
                }

                plan().emplace<physical::HomomorphicUniformAngularUnion>(plan().lookup(node), physical_outputs, 4, 4);
                return true;
            } else if(FindHomomorphicAncestor(node).has_value() &&
                      FindEncodeParent(node).has_value()) {
                auto href = FindHomomorphicAncestor(node).value();
                auto eref = FindEncodeParent(node).value();

                href->parents().emplace_back(eref);
                plan().assign(node, href);
                plan().assign(node, eref);

                plan().emplace<physical::CPUIdentity>(plan().lookup(node), href);
                return true;
            } else
                return false;
        }
    };

    class ChooseMap : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::TransformedLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecode>() || physical_parents[0].is<physical::CPUtoGPUTransfer>()
                                   ? physical_parents[0]
                                   : physical_parents[physical_parents.size() - 1];

            if(!plan().has_physical_assignment(node)) {
                if(!node.functor()->has_implementation(physical::DeviceType::GPU)) {
                    auto transfer = plan().emplace<physical::GPUtoCPUTransfer>(plan().lookup(node), hardcoded_parent);
                    plan().emplace<physical::CPUMap>(plan().lookup(node), transfer, *node.functor());
                } else
                    plan().emplace<physical::GPUMap>(plan().lookup(node), hardcoded_parent, *node.functor());
                return true;
            }
            return false;
        }
    };

    class ChooseSelection : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::SubsetLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecode>() || physical_parents[0].is<physical::CPUtoGPUTransfer>()
                                    ? physical_parents[0]
                                    : physical_parents[physical_parents.size() - 1];

            if(!plan().has_physical_assignment(node)) {
                if(hardcoded_parent->device() != physical::DeviceType::GPU)
                    throw std::runtime_error("Hardcoded support only for GPU selection"); //TODO
                LOG(ERROR) << "Assuming angular selection without actually checking";
                plan().emplace<physical::GPUAngularSubframe>(plan().lookup(node), hardcoded_parent);
                return true;
            }
            return false;
        }
    };

    class ChooseInterpolate : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::InterpolatedLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecode>() || physical_parents[0].is<physical::CPUtoGPUTransfer>()
                                    ? physical_parents[0]
                                    : physical_parents[physical_parents.size() - 1];

            if(!plan().has_physical_assignment(node)) {
                if(hardcoded_parent->device() != physical::DeviceType::GPU)
                    throw std::runtime_error("Hardcoded support only for GPU interpolation"); //TODO
                //if(!node.interpolator()->has_implementation(physical::DeviceType::GPU)) {
                plan().emplace<physical::GPUInterpolate>(plan().lookup(node), hardcoded_parent, node.interpolator());
                return true;
            }
            return false;
        }
    };

    class ChooseDiscretize : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::DiscreteLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if (physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecode>() ||
                                    physical_parents[0].is<physical::CPUtoGPUTransfer>()
                                    ? physical_parents[0]
                                    : physical_parents[physical_parents.size() - 1];
            auto is_discrete = (hardcoded_parent.is<physical::GPUDecode>() &&
                                hardcoded_parent->logical().is<logical::ScannedLightField>()) ||
                               hardcoded_parent.is<physical::GPUDownsampleResolution>();

            if (!plan().has_physical_assignment(node) && is_discrete &&
                hardcoded_parent.is<physical::GPUDownsampleResolution>()) {
                auto &discrete = node.downcast<logical::DiscreteLightField>();
                hardcoded_parent.downcast<physical::GPUDownsampleResolution>().geometries().push_back(static_cast<IntervalGeometry&>(*discrete.geometry()));
                plan().emplace<physical::CPUIdentity>(plan().lookup(node), hardcoded_parent);
            } else if(!plan().has_physical_assignment(node) && is_discrete) {
                auto downsampled = hardcoded_parent->logical().try_downcast<logical::DiscretizedLightField>();
                auto scanned = downsampled.has_value() ? hardcoded_parent->logical()->parents()[0].downcast<logical::ScannedLightField>() : hardcoded_parent->logical().downcast<logical::ScannedLightField>();
                auto &parent_geometry = scanned.metadata().geometry();
                auto &discrete_geometry = node.geometry();

                if(scanned.metadata().streams().size() != 1)
                    return false;
                else if(!discrete_geometry.is<IntervalGeometry>())
                    return false;
                else if(!parent_geometry.is<EquirectangularGeometry>())
                    return false;
                else if((discrete_geometry.downcast<IntervalGeometry>().dimension() == Dimension::Theta &&
                         scanned.metadata().streams()[0].configuration().width % discrete_geometry.downcast<IntervalGeometry>().size().value_or(1) == 0) ||
                        (discrete_geometry.downcast<IntervalGeometry>().dimension() == Dimension::Phi &&
                         scanned.metadata().streams()[0].configuration().height % discrete_geometry.downcast<IntervalGeometry>().size().value_or(1) == 0))
                {
                    if(hardcoded_parent->device() == physical::DeviceType::GPU)
                    {
                        plan().emplace<physical::GPUDownsampleResolution>(plan().lookup(node), hardcoded_parent, discrete_geometry.downcast<IntervalGeometry>());
                        return true;
                    }
                }
                //TODO handle case where interval is equal to resolution (by applying identity)
            }
            return false;
        }
    };

    class ChooseLinearScale : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::DiscreteLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecode>() || physical_parents[0].is<physical::CPUtoGPUTransfer>()
                                    ? physical_parents[0]
                                    : physical_parents[physical_parents.size() - 1];
            auto hardcoded_grandparent = hardcoded_parent->parents()[0];
            auto hardcoded_greatgrandparent = hardcoded_grandparent->parents()[0];
            auto is_linear_interpolated =
                    hardcoded_parent.is<physical::GPUInterpolate>() &&
                    hardcoded_parent->logical().downcast<logical::InterpolatedLightField>().interpolator()->name() == "linear";

            if(!plan().has_physical_assignment(node) && is_linear_interpolated) {
                auto scanned = hardcoded_grandparent->logical().is<logical::ScannedLightField>() ? hardcoded_grandparent->logical().downcast<logical::ScannedLightField>() : hardcoded_greatgrandparent->logical().downcast<logical::ScannedLightField>();
                auto &scanned_geometry = scanned.metadata().geometry();
                auto &discrete_geometry = node.geometry();

                if(scanned.metadata().streams().size() != 1)
                    return false;
                else if(!discrete_geometry.is<IntervalGeometry>())
                    return false;
                else if(!scanned_geometry.is<EquirectangularGeometry>())
                    return false;
                else if(discrete_geometry.downcast<IntervalGeometry>().dimension() == Dimension::Theta ||
                        discrete_geometry.downcast<IntervalGeometry>().dimension() == Dimension::Phi)
                {
                    if(hardcoded_parent->device() == physical::DeviceType::GPU)
                    {
                        plan().emplace<physical::GPUDownsampleResolution>(plan().lookup(node), hardcoded_parent, discrete_geometry.downcast<IntervalGeometry>());
                        return true;
                    }
                }
                //TODO handle case where interval is equal to resolution (by applying identity)
            }
            return false;
        }
    };

    class ChoosePartition : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::PartitionedLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[physical_parents.size() - 1];

            if(!plan().has_physical_assignment(node)) {
                if(hardcoded_parent->device() == physical::DeviceType::CPU)
                    plan().emplace<physical::CPUIdentity>(plan().lookup(node), hardcoded_parent);
                else if(hardcoded_parent.is<physical::GPUOperator>())
                    plan().emplace<physical::GPUIdentity>(plan().lookup(node), hardcoded_parent);
                return true;
            }
            return false;
        }
    };

    class ChooseSubquery : public OptimizerRule {
    public:
        bool visit(const logical::SubqueriedLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0];

            if(!plan().has_physical_assignment(node)) {
                plan().emplace<physical::GPUAngularSubquery>(plan().lookup(node), hardcoded_parent,
                                                             plan().environment());
                return true;
            }
            return false;
        }
    };

    class ChooseStore : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        PhysicalLightFieldReference Encode(const logical::StoredLightField &node, PhysicalLightFieldReference parent) {
            if(parent.is<physical::GPUAngularSubquery>() && parent.downcast<physical::GPUAngularSubquery>().subqueryType().is<logical::EncodedLightField>()) {
                return plan().emplace<physical::CPUIdentity>(plan().lookup(node), parent);
            } else if(parent.is<physical::GPUOperator>()) {
                return plan().emplace<physical::GPUEncode>(plan().lookup(node), parent, Codec::hevc());
            } else if(parent.is<physical::CPUMap>() && parent.downcast<physical::CPUMap>().transform()(physical::DeviceType::CPU).codec().name() == node.codec().name()) {
                return plan().emplace<physical::CPUIdentity>(plan().lookup(node), parent);
            } else {
                auto gpu = plan().environment().gpus()[0];
                auto transfer = plan().emplace<physical::CPUtoGPUTransfer>(plan().lookup(node), parent, gpu);
                return plan().emplace<physical::GPUEncode>(plan().lookup(node), transfer, Codec::hevc());
            }
        }

        bool visit(const logical::StoredLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecode>()
                                    ? physical_parents[0]
                                    : physical_parents[physical_parents.size() - 1];

            if(!plan().has_physical_assignment(node)) {
                LOG(WARNING) << "Randomly picking HEVC as codec";

                auto encode = Encode(node, hardcoded_parent);
                plan().emplace<physical::Store>(plan().lookup(node), encode);
                return true;
            }
            return false;
        }
    };
}

#endif //LIGHTDB_RULES_H
