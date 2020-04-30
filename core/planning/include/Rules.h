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
#include "SaveOperators.h"
#include "SinkOperators.h"
#include "Rectangle.h"

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
                //plan().emplace<physical::GPUOperatorAdapter>(mref);
                // Made this change without testing it -- when is this rule fired?
                plan().emplace<physical::MaterializedToPhysicalOperatorAdapter>(ref, mref);
                return true;
            } else if(node.is<physical::PhysicalToLogicalLightFieldAdapter>()) {
                auto ref = plan().lookup(node);
                //auto op = plan().emplace<physical::GPUOperatorAdapter>(ref.downcast<physical::PhysicalToLogicalLightFieldAdapter>().source());
                //plan().assign(ref, op);
                plan().assign(ref, ref.downcast<physical::PhysicalToLogicalLightFieldAdapter>().source());
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
                LOG(ERROR) << "Applying decode rule";
                if(node.entry().sources().empty())
                    LOG(WARNING) << "Attempt to decode a catalog entry with no underlying streams";

                for(const auto &stream: node.entry().sources()) {
                    //auto stream = node.entry().streams()[0];
                    auto logical = plan().lookup(node);

                    if((stream.codec() == Codec::h264() ||
                        stream.codec() == Codec::hevc()) &&
                       !plan().environment().gpus().empty()) {
                        auto gpu = plan().allocator().gpu();
                        //auto gpu = plan().environment().gpus()[0];

                        auto &scan = plan().emplace<physical::ScanSingleFileDecodeReader>(logical, stream);
                        auto decode = plan().emplace<physical::GPUDecodeFromCPU>(logical, scan, gpu);

                        auto children = plan().children(plan().lookup(node));
                        if (children.size() > 1) {
                            auto tees = physical::TeedPhysicalOperatorAdapter::make(decode, children.size());
                            for (auto index = 0u; index < children.size(); index++)
                                plan().add(tees->physical(index));
                            //plan().emplace<physical::GPUOperatorAdapter>(tees->physical(index), decode);
                        }
                    } else if(stream.codec() == Codec::h264() ||
                              stream.codec() == Codec::hevc()) {
                        auto &scan = plan().emplace<physical::ScanSingleFileDecodeReader>(logical, stream);
                        auto decode = plan().emplace<physical::CPUDecode>(logical, scan);

                        auto children = plan().children(plan().lookup(node));
                        if(children.size() > 1) {
                            auto tees = physical::TeedPhysicalOperatorAdapter::make(decode, children.size());
                            for (auto index = 0u; index < children.size(); index++)
                                plan().add(tees->physical(index));
                        }
                    } else if(stream.codec() == Codec::boxes()) {
                        auto &scan = plan().emplace<physical::ScanSingleFile<sizeof(Rectangle) * 8192>>(logical, stream);
                        auto decode = plan().emplace<physical::CPUFixedLengthRecordDecode<Rectangle>>(logical, scan);
                    } else
                        throw NotImplementedError("Unsupported codec when assigning decoder");
                }

                return true;
            }
            return false;
        }

        bool visit(const logical::ExternalLightField &node) override {
            if(!plan().has_physical_assignment(node)) {
                auto logical = plan().lookup(node);

                if(node.codec() == Codec::h264() ||
                   node.codec() == Codec::hevc()) {
                    //auto gpu = plan().environment().gpus()[0];
                    auto gpu = plan().allocator().gpu();

                    auto &scan = plan().emplace<physical::ScanSingleFileDecodeReader>(logical, node.source());
                    auto decode = plan().emplace<physical::GPUDecodeFromCPU>(logical, scan, gpu);

                    auto children = plan().children(plan().lookup(node));
                    if(children.size() > 1) {
                        auto tees = physical::TeedPhysicalOperatorAdapter::make(decode, children.size());
                        for (auto index = 0u; index < children.size(); index++)
                            plan().add(tees->physical(index));
                            //plan().emplace<physical::GPUOperatorAdapter>(tees->physical(index), decode);
                    }
                } else if(node.codec() == Codec::boxes()) {
                    auto &scan = plan().emplace<physical::ScanSingleFile<sizeof(Rectangle) * 8192>>(logical, node.source());
                    auto decode = plan().emplace<physical::CPUFixedLengthRecordDecode<Rectangle>>(logical, scan);
                }

                return true;
            }
            return false;
        }
    };

    class ChooseEncoders : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::EncodedLightField &node) override {
            if(!plan().has_physical_assignment(node)) {
                auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                        node.parents().begin(), node.parents().end(),
                        [this](auto &parent) { return plan().unassigned(parent); });

                if(physical_parents.empty())
                    return false;

                //TODO clean this up, shouldn't just be randomly picking last parent
                auto physical_parent = physical_parents[0];
                auto logical = plan().lookup(node);

                if(physical_parent.is<physical::GPUAngularSubquery>() && physical_parent.downcast<physical::GPUAngularSubquery>().subqueryType().is<logical::EncodedLightField>())
                    plan().emplace<physical::CPUIdentity>(logical, physical_parent);
                else if(physical_parent.is<physical::GPUOperator>() && node.codec().nvidiaId().has_value())
                    plan().emplace<physical::GPUEncodeToCPU>(logical, physical_parent, node.codec());
                else if(physical_parent.is<physical::GPUOperator>() && node.codec() == Codec::raw())
                    plan().emplace<physical::GPUEnsureFrameCropped>(plan().lookup(node), physical_parent);
                else if(physical_parent.is<physical::CPUMap>() && physical_parent.downcast<physical::CPUMap>().transform()(physical::DeviceType::CPU).codec().name() == node.codec().name())
                    plan().emplace<physical::CPUIdentity>(logical, physical_parent);
                //TODO this is silly -- every physical operator should declare an output type and we should just use that
                else if(physical_parent.is<physical::TeedPhysicalOperatorAdapter::TeedPhysicalOperator>() && physical_parent->parents()[0].is<physical::CPUMap>() && physical_parent->parents()[0].downcast<physical::CPUMap>().transform()(physical::DeviceType::CPU).codec().name() == node.codec().name())
                    plan().emplace<physical::CPUIdentity>(logical, physical_parent);
                else if(physical_parent->device() == physical::DeviceType::CPU) {
                    //auto gpu = plan().environment().gpus()[0];
                    auto gpu = plan().allocator().gpu();
                    auto transfer = plan().emplace<physical::CPUtoGPUTransfer>(logical, physical_parent, gpu);
                    plan().emplace<physical::GPUEncodeToCPU>(plan().lookup(node), transfer, node.codec());
                } else
                    return false;
                return true;
            } else {

            }
            return false;
        }
    };

    class ChooseUnion : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        std::optional<PhysicalOperatorReference> FindEncodeParent(const LightFieldReference &node) {
            for(const auto &parent: node->parents()) {
                if(parent.is<logical::EncodedLightField>() &&
                   plan().has_physical_assignment(parent))
                    return {plan().assignments(parent).front()};
            }
            return {};
        }

        std::optional<PhysicalOperatorReference> FindHomomorphicAncestor(const LightFieldReference &node) {
            std::deque<PhysicalOperatorReference> queue;

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

        bool TryGPUBoxOverlayUnion(const logical::CompositeLightField &node) {
            auto leafs0 = plan().unassigned(node.parents()[0]);
            auto leafs1 = plan().unassigned(node.parents()[1]);

            /*if(leafs0.size() == 2 && leafs1.empty()) {
                leafs1.push_back(leafs0[1]);
                leafs0.pop_back();
            }*/

            if(leafs0.size() != 1 || leafs1.size() != 1)
                return false;
            //TODO shouldn't arbitrarily require a shallow union
            else if(!node.parents()[0].is<logical::ScannedLightField>() || !node.parents()[1].is<logical::ScannedLightField>())
                return false;
            //TODO should pay attention to all streams
            else if(node.parents()[0].downcast<logical::ScannedLightField>().entry().sources()[0].codec() != Codec::boxes())
                return false;
            else if(node.parents()[1].downcast<logical::ScannedLightField>().entry().sources()[0].codec() != Codec::h264() &&
                    node.parents()[1].downcast<logical::ScannedLightField>().entry().sources()[0].codec() != Codec::hevc())
                return false;
            else {
                auto unioned = plan().emplace<physical::GPUBoxOverlayUnion>(
                        plan().lookup(node),
                        std::vector<PhysicalOperatorReference>{leafs0[0], leafs1[0]});

                auto children = plan().children(plan().lookup(node));
                if(children.size() > 1) {
                    auto tees = physical::TeedPhysicalOperatorAdapter::make(unioned, children.size());
                    for (auto index = 0u; index < children.size(); index++) {
                        if (unioned->device() == physical::DeviceType::CPU)
                            plan().assign(plan().lookup(node), tees->physical(index));
                        else if (unioned->device() == physical::DeviceType::GPU)
                            plan().add(tees->physical(index));
                            //plan().emplace<physical::GPUOperatorAdapter>(tees->physical(index), unioned);
                        else
                            throw InvalidArgumentError("No rule support for device type.", "node");
                    }
                }
                
                return true;
            }
        }

        bool visit(const logical::CompositeLightField &node) override {
            if (plan().has_physical_assignment(node))
                return false;
            else if (node.parents().size() != 2)
                return false;
            else if (node.parents()[0].is<logical::EncodedLightField>() &&
                     node.parents()[1].is<logical::EncodedLightField>()) {
                std::vector<PhysicalOperatorReference> physical_outputs;
                for (auto &parent: node.parents()) {
                    const auto &assignments = plan().assignments(parent);
                    if (assignments.empty())
                        return false;
                    //physical_outputs.push_back(assignments[assignments.size() - 1]);
                    physical_outputs.push_back(assignments.back());
                }

                plan().emplace<physical::HomomorphicUniformAngularUnion>(plan().lookup(node), physical_outputs, 4, 4);
                return true;
            } else if (FindHomomorphicAncestor(node).has_value() &&
                       FindEncodeParent(node).has_value()) {
                auto href = FindHomomorphicAncestor(node).value();
                auto eref = FindEncodeParent(node).value();

                href->parents().emplace_back(eref);
                plan().assign(node, href);
                plan().assign(node, eref);

                plan().emplace<physical::CPUIdentity>(plan().lookup(node), href);
                return true;
            } else if(TryGPUBoxOverlayUnion(node)) {
                return true;
            } else
                return false;
        }
    };

    class ChooseMap : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        PhysicalOperatorReference Map(const logical::TransformedLightField &node, PhysicalOperatorReference parent) {
            auto logical = plan().lookup(node);

            if(!node.functor()->has_implementation(physical::DeviceType::GPU)) {
                auto transfer = plan().emplace<physical::GPUtoCPUTransfer>(plan().lookup(node), parent);
                return plan().emplace<physical::CPUMap>(plan().lookup(node), transfer, *node.functor());
            } else
                return plan().emplace<physical::GPUMap>(plan().lookup(node), parent, *node.functor());
        }


        bool visit(const logical::TransformedLightField &node) override {
            if(!plan().has_physical_assignment(node)) {
                auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                        node.parents().begin(), node.parents().end(),
                        [this](auto &parent) { return plan().unassigned(parent); });

                if(!physical_parents.empty()) {
                    auto mapped = Map(node, physical_parents[0]);

                    //TODO what if function isn't determistic?!

                    auto children = plan().children(plan().lookup(node));
                    if(children.size() > 1) {
                        auto tees = physical::TeedPhysicalOperatorAdapter::make(mapped, children.size());
                        for(auto index = 0u; index < children.size(); index++) {
                            if(mapped->device() == physical::DeviceType::CPU)
                                plan().assign(plan().lookup(node), tees->physical(index));
                            else if(mapped->device() == physical::DeviceType::GPU)
                                plan().add(tees->physical(index));
                                //plan().emplace<physical::GPUOperatorAdapter>(tees->physical(index), mapped);
                            else
                                throw InvalidArgumentError("No rule support for device type.", "node");
                        }
                    }
                    return true;
                }
            }
            return false;
        }
    };

    class ChooseSelection : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        PhysicalOperatorReference AngularSelection(const logical::SubsetLightField &node,
                                                     PhysicalOperatorReference parent) {
            if(parent->device() == physical::DeviceType::GPU)
                return plan().emplace<physical::GPUAngularSubframe>(plan().lookup(node), parent);
            else
                throw std::runtime_error("Hardcoded support only for GPU angular selection"); //TODO
        }

        PhysicalOperatorReference TemporalSelection(const logical::SubsetLightField &node,
                                                      PhysicalOperatorReference parent) {
            LOG(WARNING) << "Assuming temporal selection parent is encoded video without actually checking";
            if(parent->device() == physical::DeviceType::GPU)
                return plan().emplace<physical::FrameSubset>(plan().lookup(node), parent);
            else
                throw std::runtime_error("Hardcoded support only for GPU temporal selection"); //TODO
        }

        PhysicalOperatorReference IdentitySelection(const logical::SubsetLightField &node,
                                                      PhysicalOperatorReference parent) {
            if(parent->device() == physical::DeviceType::CPU)
                return plan().emplace<physical::CPUIdentity>(plan().lookup(node), parent);
            else if(parent->device() == physical::DeviceType::GPU)
                return plan().emplace<physical::GPUIdentity>(plan().lookup(node), parent);
            else
                throw std::runtime_error("No identity support for FPGA"); //TODO
        }

        bool visit(const logical::SubsetLightField &node) override {
            if(!plan().has_physical_assignment(node)) {
                auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                        node.parents().begin(), node.parents().end(),
                        [this](auto &parent) { return plan().unassigned(parent); });

                if(physical_parents.empty())
                    return false;

                auto selection = physical_parents[0];
                auto dimensions = node.dimensions();

                selection = dimensions.find(Dimension::Theta) != dimensions.end() ||
                            dimensions.find(Dimension::Phi) != dimensions.end()
                    ? AngularSelection(node, selection)
                    : selection;

                selection = dimensions.find(Dimension::Time) != dimensions.end()
                    ? TemporalSelection(node, selection)
                    : selection;

                selection = dimensions.empty()
                    ? IdentitySelection(node, selection)
                    : selection;

                if(dimensions.find(Dimension::X) != dimensions.end() ||
                        dimensions.find(Dimension::Y) != dimensions.end() ||
                        dimensions.find(Dimension::Z) != dimensions.end())
                    throw std::runtime_error("Missing support for spatial selection"); //TODO

                return selection != physical_parents[0];
            }
            return false;
        }
    };

    class ChooseInterpolate : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::InterpolatedLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecodeFromCPU>() || physical_parents[0].is<physical::CPUtoGPUTransfer>()
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
        void teeIfNecessary(const LightField& node, PhysicalOperatorReference physical) {
            auto children = plan().children(plan().lookup(node));
            if(children.size() > 1) {
                auto tees = physical::TeedPhysicalOperatorAdapter::make(physical, children.size());
                for (auto index = 0u; index < children.size(); index++) {
                    if (physical->device() == physical::DeviceType::CPU)
                        plan().assign(plan().lookup(node), tees->physical(index));
                    else if (physical->device() == physical::DeviceType::GPU)
                        plan().add(tees->physical(index));
                        //plan().emplace<physical::GPUOperatorAdapter>(tees->physical(index), physical);
                    else
                        throw InvalidArgumentError("No rule support for device type.", "node");
                }
            }
        }


    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::DiscreteLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if (physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecodeFromCPU>() ||
                                    physical_parents[0].is<physical::CPUtoGPUTransfer>()
                                    ? physical_parents[0]
                                    : physical_parents[physical_parents.size() - 1];
            auto is_discrete = (hardcoded_parent.is<physical::GPUDecodeFromCPU>() &&
                                hardcoded_parent->logical().is<logical::ScannedLightField>()) ||
                               hardcoded_parent.is<physical::GPUDownsampleResolution>();

            if (!plan().has_physical_assignment(node) && is_discrete &&
                hardcoded_parent.is<physical::GPUDownsampleResolution>()) {
                auto &discrete = node.downcast<logical::DiscreteLightField>();
                hardcoded_parent.downcast<physical::GPUDownsampleResolution>().geometries().push_back(static_cast<IntervalGeometry&>(*discrete.geometry()));
                // Was CPUIdentity, bug or intentional?
                auto identity = plan().emplace<physical::GPUIdentity>(plan().lookup(node), hardcoded_parent);
                teeIfNecessary(node, identity);
                return true;
            } else if(!plan().has_physical_assignment(node) && is_discrete) {
                auto downsampled = hardcoded_parent->logical().try_downcast<logical::DiscretizedLightField>();
                auto scanned = downsampled.has_value() ? hardcoded_parent->logical()->parents()[0].downcast<logical::ScannedLightField>() : hardcoded_parent->logical().downcast<logical::ScannedLightField>();
                auto &parent_geometry = scanned.entry().sources().front().geometry();
                auto &discrete_geometry = node.geometry();

                if(scanned.entry().sources().size() != 1)
                    return false;
                else if(!discrete_geometry.is<IntervalGeometry>())
                    return false;
                else if(!parent_geometry.is<EquirectangularGeometry>())
                    return false;
                else if((discrete_geometry.downcast<IntervalGeometry>().dimension() == Dimension::Theta &&
                         scanned.entry().sources()[0].configuration().width % discrete_geometry.downcast<IntervalGeometry>().size().value_or(1) == 0) ||
                        (discrete_geometry.downcast<IntervalGeometry>().dimension() == Dimension::Phi &&
                         scanned.entry().sources()[0].configuration().height % discrete_geometry.downcast<IntervalGeometry>().size().value_or(1) == 0))
                {
                    if(hardcoded_parent->device() == physical::DeviceType::GPU)
                    {
                        auto downsample = plan().emplace<physical::GPUDownsampleResolution>(plan().lookup(node), hardcoded_parent, discrete_geometry.downcast<IntervalGeometry>());
                        teeIfNecessary(node, downsample);
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
            auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0].is<physical::GPUDecodeFromCPU>() || physical_parents[0].is<physical::CPUtoGPUTransfer>()
                                    ? physical_parents[0]
                                    : physical_parents[physical_parents.size() - 1];
            auto hardcoded_grandparent = hardcoded_parent->parents()[0];
            auto hardcoded_greatgrandparent = hardcoded_grandparent->parents()[0];
            auto is_linear_interpolated =
                    hardcoded_parent.is<physical::GPUInterpolate>() &&
                    hardcoded_parent->logical().downcast<logical::InterpolatedLightField>().interpolator()->name() == "linear";

            if(!plan().has_physical_assignment(node) && is_linear_interpolated) {
                auto scanned = hardcoded_grandparent->logical().is<logical::ScannedLightField>() ? hardcoded_grandparent->logical().downcast<logical::ScannedLightField>() : hardcoded_greatgrandparent->logical().downcast<logical::ScannedLightField>();
                auto &scanned_geometry = scanned.entry().sources().front().geometry();
                auto &discrete_geometry = node.geometry();

                if(scanned.entry().sources().size() != 1)
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
            auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            //OMG this is even more horrible now
            size_t index;
            if(physical_parents.size() == 1)
                index = 0;
            else if(physical_parents.size() == 2) {
                // Pick the most parentiest of parents, wtf
                if(!physical_parents[0]->parents().empty() && physical_parents[0]->parents()[0] == physical_parents[1])
                    index = 0;
                else
                    index = 1;
            } else
                index = physical_parents.size() - 1;
            //auto hardcoded_parent = physical_parents[physical_parents.size() - 1];
            auto hardcoded_parent = physical_parents[index];

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
        void teeIfNecessary(const LightField& node, const PhysicalOperatorReference &physical) {
            auto children = plan().children(plan().lookup(node));
            if(children.size() > 1) {
                auto tees = physical::TeedPhysicalOperatorAdapter::make(physical, children.size());
                for (auto index = 0u; index < children.size(); index++) {
                    plan().assign(plan().lookup(node), tees->physical(index));
                    /*if (physical->device() == physical::DeviceType::CPU)
                        plan().assign(plan().lookup(node), tees->physical(index));
                    else if (physical->device() == physical::DeviceType::GPU)
                        plan().emplace<physical::GPUOperatorAdapter>(tees->physical(index), physical);
                    else
                        throw InvalidArgumentError("No rule support for device type.", "node");*/
                }
            }
        }

    public:
        explicit ChooseSubquery(const OptimizerReference &optimizer)
            : optimizer_(optimizer)
        { }

        bool visit(const logical::SubqueriedLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(physical_parents.empty())
                return false;

            //TODO clean this up, shouldn't just be randomly picking last parent
            auto hardcoded_parent = physical_parents[0];

            if(!plan().has_physical_assignment(node)) {
                auto subquery = plan().emplace<physical::GPUAngularSubquery>(
                        plan().lookup(node), hardcoded_parent, optimizer_);
                teeIfNecessary(node, subquery);
                return true;
            }
            return false;
        }

    private:
        const OptimizerReference optimizer_;
    };

    class ChooseStore : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        PhysicalOperatorReference Encode(const logical::StoredLightField &node, PhysicalOperatorReference parent) {
            auto logical = plan().lookup(node);

            // Can we leverage the ChooseEncode rule to automatically do this stuff, which is an exact duplicate?

            if(parent.is<physical::GPUAngularSubquery>() && parent.downcast<physical::GPUAngularSubquery>().subqueryType().is<logical::EncodedLightField>()) {
                return plan().emplace<physical::CPUIdentity>(logical, parent);
            //} else if(parent.is<physical::GPUOperatorAdapter>() && parent->parents()[0].is<physical::GPUAngularSubquery>() && parent->parents()[0].downcast<physical::GPUAngularSubquery>().subqueryType().is<logical::EncodedLightField>()) {
            //    return plan().emplace<physical::CPUIdentity>(logical, parent);
            } else if(parent.is<physical::GPUEncodeToCPU>()) {
                return plan().emplace<physical::GPUIdentity>(logical, parent);
            } else if(parent.is<physical::GPUOperator>()) {
                return plan().emplace<physical::GPUEncodeToCPU>(logical, parent, Codec::hevc());
            } else if(parent.is<physical::CPUMap>() && parent.downcast<physical::CPUMap>().transform()(physical::DeviceType::CPU).codec().name() == node.codec().name()) {
                return plan().emplace<physical::CPUIdentity>(logical, parent);
                //TODO this is silly -- every physical operator should declare an output type and we should just use that
            } else if(parent.is<physical::TeedPhysicalOperatorAdapter::TeedPhysicalOperator>() && parent->parents()[0].is<physical::CPUMap>() && parent->parents()[0].downcast<physical::CPUMap>().transform()(physical::DeviceType::CPU).codec().name() == node.codec().name()) {
                return plan().emplace<physical::CPUIdentity>(logical, parent);
            } else if(parent.is<physical::TeedPhysicalOperatorAdapter::TeedPhysicalOperator>() && parent->parents()[0].is<physical::GPUAngularSubquery>()) {
                return plan().emplace<physical::CPUIdentity>(logical, parent);
            } else if(parent->device() != physical::DeviceType::GPU) {
                //auto gpu = plan().environment().gpus()[0];
                auto gpu = plan().allocator().gpu();
                auto transfer = plan().emplace<physical::CPUtoGPUTransfer>(logical, parent, gpu);
                return plan().emplace<physical::GPUEncodeToCPU>(logical, transfer, Codec::hevc());
            } else if(!parent.is<physical::GPUOperator>()) {
                //auto gpuop = plan().emplace<physical::GPUOperatorAdapter>(parent);
                //return plan().emplace<physical::GPUEncodeToCPU>(logical, gpuop, Codec::hevc());
                return plan().emplace<physical::GPUEncodeToCPU>(logical, parent, Codec::hevc());
            } else
                return plan().emplace<physical::GPUEncodeToCPU>(logical, parent, Codec::hevc());
        }

        bool visit(const logical::StoredLightField &node) override {
            if(!plan().has_physical_assignment(node)) {
                auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                        node.parents().begin(), node.parents().end(),
                        [this](auto &parent) { return plan().unassigned(parent); });

                if(physical_parents.empty())
                    return false;

                auto encode = Encode(node, physical_parents[0]);
                plan().emplace<physical::Store>(plan().lookup(node), encode);
                return true;
            }
            return false;
        }
    };

    class ChooseSink : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        PhysicalOperatorReference Encode(const logical::SunkLightField &node, PhysicalOperatorReference parent) {
            auto logical = plan().lookup(node);

            // Can we leverage the ChooseEncode rule to automatically do this stuff, which is an exact duplicate?
            //TODO Copied from ChooseStore, which is lame

            if(parent.is<physical::GPUAngularSubquery>() && parent.downcast<physical::GPUAngularSubquery>().subqueryType().is<logical::EncodedLightField>()) {
                return plan().emplace<physical::CPUIdentity>(logical, parent);
                //} else if(parent.is<physical::GPUOperatorAdapter>() && parent->parents()[0].is<physical::GPUAngularSubquery>() && parent->parents()[0].downcast<physical::GPUAngularSubquery>().subqueryType().is<logical::EncodedLightField>()) {
                //    return plan().emplace<physical::CPUIdentity>(logical, parent);
            } else if(parent.is<physical::GPUEncodeToCPU>()) {
                return plan().emplace<physical::GPUIdentity>(logical, parent);
            } else if(parent.is<physical::GPUOperator>()) {
                return plan().emplace<physical::GPUEncodeToCPU>(logical, parent, Codec::hevc());
                //TODO this is silly -- every physical operator should declare an output type and we should just use that
            } else if(parent.is<physical::TeedPhysicalOperatorAdapter::TeedPhysicalOperator>() && parent->parents()[0].is<physical::GPUAngularSubquery>()) {
                return plan().emplace<physical::CPUIdentity>(logical, parent);
            } else if(parent->device() != physical::DeviceType::GPU) {
                //auto gpu = plan().environment().gpus()[0];
                auto gpu = plan().allocator().gpu();
                auto transfer = plan().emplace<physical::CPUtoGPUTransfer>(logical, parent, gpu);
                return plan().emplace<physical::GPUEncodeToCPU>(logical, transfer, Codec::hevc());
            } else if(!parent.is<physical::GPUOperator>()) {
                //auto gpuop = plan().emplace<physical::GPUOperatorAdapter>(parent);
                //return plan().emplace<physical::GPUEncodeToCPU>(logical, gpuop, Codec::hevc());
                return plan().emplace<physical::GPUEncodeToCPU>(logical, parent, Codec::hevc());
            } else
                return plan().emplace<physical::GPUEncodeToCPU>(logical, parent, Codec::hevc());
        }

        bool visit(const logical::SunkLightField &node) override {
            if(!plan().has_physical_assignment(node)) {
                auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                        node.parents().begin(), node.parents().end(),
                        [this](auto &parent) { return plan().unassigned(parent); });

                if(physical_parents.empty())
                    return false;

                auto sink = Encode(node, physical_parents.front());
                plan().emplace<physical::Sink>(plan().lookup(node), sink);
                return true;
            }
            return false;
        }
    };

    class ChooseSave : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        //TODO Duplicate of ChooseStore::Encode, this is lame
        PhysicalOperatorReference Encode(const logical::SavedLightField &node, PhysicalOperatorReference parent) {
            auto logical = plan().lookup(node);

            // Can we leverage the ChooseEncode rule to automatically do this stuff, which is an exact duplicate?

            if(parent.is<physical::GPUAngularSubquery>() && parent.downcast<physical::GPUAngularSubquery>().subqueryType().is<logical::EncodedLightField>()) {
                return plan().emplace<physical::CPUIdentity>(logical, parent);
                //} else if(parent.is<physical::GPUOperatorAdapter>() && parent->parents()[0].is<physical::GPUAngularSubquery>() && parent->parents()[0].downcast<physical::GPUAngularSubquery>().subqueryType().is<logical::EncodedLightField>()) {
                //    return plan().emplace<physical::CPUIdentity>(logical, parent);
            } else if(parent.is<physical::GPUEncodeToCPU>()) {
                return plan().emplace<physical::GPUIdentity>(logical, parent);
            } else if(parent.is<physical::GPUOperator>()) {
                return plan().emplace<physical::GPUEncodeToCPU>(logical, parent, Codec::hevc());
            } else if(parent.is<physical::CPUMap>() && parent.downcast<physical::CPUMap>().transform()(physical::DeviceType::CPU).codec().name() == node.codec().name()) {
                return plan().emplace<physical::CPUIdentity>(logical, parent);
                //TODO this is silly -- every physical operator should declare an output type and we should just use that
            } else if(parent.is<physical::TeedPhysicalOperatorAdapter::TeedPhysicalOperator>() && parent->parents()[0].is<physical::CPUMap>() && parent->parents()[0].downcast<physical::CPUMap>().transform()(physical::DeviceType::CPU).codec().name() == node.codec().name()) {
                return plan().emplace<physical::CPUIdentity>(logical, parent);
            } else if(parent.is<physical::TeedPhysicalOperatorAdapter::TeedPhysicalOperator>() && parent->parents()[0].is<physical::GPUAngularSubquery>()) {
                return plan().emplace<physical::CPUIdentity>(logical, parent);
            } else if(parent->device() != physical::DeviceType::GPU) {
                //auto gpu = plan().environment().gpus()[0];
                auto gpu = plan().allocator().gpu();
                auto transfer = plan().emplace<physical::CPUtoGPUTransfer>(logical, parent, gpu);
                return plan().emplace<physical::GPUEncodeToCPU>(logical, transfer, Codec::hevc());
            } else if(!parent.is<physical::GPUOperator>()) {
                //auto gpuop = plan().emplace<physical::GPUOperatorAdapter>(parent);
                //return plan().emplace<physical::GPUEncodeToCPU>(logical, gpuop, Codec::hevc());
                return plan().emplace<physical::GPUEncodeToCPU>(logical, parent, Codec::hevc());
            } else
                return plan().emplace<physical::GPUEncodeToCPU>(logical, parent, Codec::hevc());
        }

        bool visit(const logical::SavedLightField &node) override {
            if(!plan().has_physical_assignment(node)) {
                auto physical_parents = functional::flatmap<std::vector<PhysicalOperatorReference>>(
                        node.parents().begin(), node.parents().end(),
                        [this](auto &parent) { return plan().unassigned(parent); });

                if(physical_parents.empty())
                    return false;

                auto encode = Encode(node, physical_parents.front());
                plan().emplace<physical::SaveToFile>(plan().lookup(node), encode);
                return true;
            }
            return false;
        }
    };

    class RemoveIdentities : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const LightField &node) override {
            auto logical = plan().lookup(node);
            auto assignments = plan().assignments(logical);

            for(auto &assignment: assignments) {
                if(assignment.is<physical::GPUIdentity>() ||
                   assignment.is<physical::CPUIdentity>()) {
                    auto &parents = assignment->parents();

                    CHECK_EQ(parents.size(), 1);

                    auto parent = parents.at(0);

                    parents.clear();
                    plan().replace_assignments(assignment, parent);

                    return true;
                }
            }

            return false;
        }
    };

    class RemoveDegenerateDecodeEncode: public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool IsParentADecodeOperator(const LightField &node) {
            return node.parents().size() == 1 &&
                   plan().assignments(node.parents().front()).size() == 1 &&
                   plan().assignments(node.parents().front()).front().is<physical::GPUDecodeFromCPU>();
        }

        //TODO Uh, implement this
        bool IsCompatibleCodecConfiguration() {
            return true;
        }

        PhysicalOperatorReference CreateIdentity(const LightFieldReference& logical, PhysicalOperatorReference &parent) {
            if(parent->device() == physical::DeviceType::CPU)
                return plan().emplace<physical::CPUIdentity>(logical, parent);
            else if(parent->device() == physical::DeviceType::GPU)
                return plan().emplace<physical::GPUIdentity>(logical, parent);
            else
                throw InvalidArgumentError("Unsupported device type for identity creation", "parent");
        }

        bool visit(const logical::EncodedLightField &node) override {
            auto logical = plan().lookup(node);

            if(IsParentADecodeOperator(*logical) &&
               IsCompatibleCodecConfiguration()) {
                auto &encode = logical;
                auto &decode = logical->parents().front();
                auto out = plan().children(logical);

                auto decode_physical = plan().assignments(decode).front();
                auto encode_physical = plan().assignments(logical).front();
                auto source_physical = functional::single(decode_physical->parents()); //plan().assignments(source).front();

                auto decode_identity = CreateIdentity(decode, source_physical);
                auto encode_identity = CreateIdentity(encode, decode_identity);

                // Remove encode
                plan().replace_assignments(encode_physical, encode_identity);

                // Remove decode
                plan().replace_assignments(decode_physical, decode_identity);

                return true;
            }

            return false;
        }
    };

    class ConvertLoadSaveToCopy: public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool IsPhysicalChildASaveOperator(const LightFieldReference &node) {
            const auto children = plan().children(node);
            return children.size() == 1 &&
                   plan().assignments(children.front()).size() == 1 &&
                   plan().assignments(children.front()).front().is<physical::ScanSingleFileDecodeReader>();
        }

        PhysicalOperatorReference CreateIdentity(const LightFieldReference& logical, PhysicalOperatorReference &parent) {
            if(parent->device() == physical::DeviceType::CPU)
                return plan().emplace<physical::CPUIdentity>(logical, parent);
            else if(parent->device() == physical::DeviceType::GPU)
                return plan().emplace<physical::GPUIdentity>(logical, parent);
            else
                throw InvalidArgumentError("Unsupported device type for identity creation", "parent");
        }

        bool ApplyRule(const LightField &node) {
            auto logical = plan().lookup(node);

            LOG(ERROR) << "Applying scan identity rule";
            if(IsPhysicalChildASaveOperator(logical)) {
                auto &scan = logical;

                auto scan_physical = plan().assignments(scan).front();
                auto save_physical =  plan().children(scan_physical).front();
                auto in_physical = scan_physical->parents();
                auto out_physical = plan().children(save_physical);

                auto &save = save_physical->logical().downcast<logical::SavedLightField>();

                auto copy_physical = plan().emplace<physical::CopyFile>(
                        scan, std::vector<std::filesystem::path>{save.filename()}, in_physical);
                auto save_identity = CreateIdentity(save, copy_physical);

                plan().replace_assignments(save_physical, save_identity);
                plan().replace_assignments(scan_physical, copy_physical);

                return true;
            }

            return false;
        }

        bool visit(const logical::ExternalLightField &node) override {
            return ApplyRule(node);
        }

        bool visit(const logical::ScannedLightField &node) override {
            return ApplyRule(node);
        }
    };
}

#endif //LIGHTDB_RULES_H
