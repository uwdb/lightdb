#ifndef LIGHTDB_RULES_H
#define LIGHTDB_RULES_H

#include "Optimizer.h"
#include "ScanOperators.h"
#include "EncodeOperators.h"
#include "DecodeOperators.h"
#include "UnionOperators.h"

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
            //TODO clean this up, shouldn't just be randomly picking last parent
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            auto physical_parent = physical_parents[0].is<physical::GPUDecode>()
                                   ? physical_parents[0]
                                   : physical_parents[physical_parents.size() - 1];

            LOG(WARNING) << "Randomly picking HEVC as codec";

            if(!plan().has_physical_assignment(node)) {
                plan().emplace<physical::GPUEncode>(plan().lookup(node), physical_parent, Codec::hevc());
                return true;
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
            LOG(WARNING) << "Randomly picking HEVC as codec";

            if(!plan().has_physical_assignment(node)) {
                plan().emplace<physical::GPUUnion>(plan().lookup(node), physical_outputs, Codec::hevc(), 2, 1);
                return true;
            }
            return false;
        }
    };
}

#endif //LIGHTDB_RULES_H
