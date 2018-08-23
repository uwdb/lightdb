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

namespace lightdb::optimization {

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
                if(hardcoded_parent.is<physical::GPUOperator>())
                    plan().emplace<physical::GPUEncode>(plan().lookup(node), hardcoded_parent, Codec::hevc());
                else if(hardcoded_parent.is<physical::CPUMap>() && hardcoded_parent.downcast<physical::CPUMap>().transform()(physical::DeviceType::CPU).codec().name() == node.codec().name())
                    plan().emplace<physical::IdentityEncode>(plan().lookup(node), hardcoded_parent);
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

        bool visit(const logical::CompositeLightField &node) override {
            //TODO this assumes a linear sequence of operators, needs some sort of dedicated "outputs" function
            //TODO better: just have a composite physical op that manages this internally and force a 1-1 mapping in plan
            //TODO clean up implementation
            std::vector<PhysicalLightFieldReference> physical_outputs;
            for(auto &parent: node.parents()) {
                auto &assignments = plan().assignments(parent);
                physical_outputs.push_back(assignments[assignments.size() - 1]);
            }

            LOG(WARNING) << "Assuming 2 rows, 1 column";

            if(!plan().has_physical_assignment(node)) {
                plan().emplace<physical::GPUUnion>(plan().lookup(node), physical_outputs, 2, 1);
                return true;
            }
            return false;
        }
    };

    class ChooseMapTransfers : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::TransformedLightField &node) override {
            /*auto gpu = plan().environment().gpus()[0];
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecode>()
                                    ? physical_parents[0]
                                    : physical_parents[physical_parents.size() - 1];

            if (!plan().has_physical_assignment(node) &&
                !node.functor()->has_implementation(hardcoded_parent->device())) {
                    switch(node.functor()->preferred_implementation().device()) {
                        case physical::DeviceType::CPU:
                            plan().emplace<physical::CPUTransfer>(plan().lookup(node),
                                                                  physical_parents[physical_parents.size() - 1]);
                            return true;

                        case physical::DeviceType::GPU:
                            LOG(WARNING) << "Using first GPU and ignoring all others";
                            plan().emplace<physical::GPUTransfer>(plan().lookup(node),
                                                                  physical_parents[physical_parents.size() - 1],
                                                                  gpu);
                            return true;

                        case physical::DeviceType::FPGA:
                        default:
                            break;
                    }
            }*/
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
                //auto gpu = plan().environment().gpus()[0];
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
                plan().emplace<physical::IdentityEncode>(plan().lookup(node), hardcoded_parent);
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
                //auto downsampled = hardcoded_parent->parents()[0]->logical().try_downcast<logical::DiscretizedLightField>();
                //auto scanned = downsampled.has_value() ? hardcoded_parent->parents()[0]->logical()->parents()[0].downcast<logical::ScannedLightField>() : hardcoded_grandparent->logical().downcast<logical::ScannedLightField>();
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

}

#endif //LIGHTDB_RULES_H
