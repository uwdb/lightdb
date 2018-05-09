#ifndef LIGHTDB_RULES_H
#define LIGHTDB_RULES_H

#include "Optimizer.h"

namespace lightdb::optimization {

    class ChooseDecoders : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::ScannedLightField &node) override {
            if(!plan().has_physical_assignment(node)) {
                LOG(WARNING) << "Just using first stream and ignoring all others";
                auto logical = plan().lookup(node);
                auto &scan = plan().emplace<physical::ScanSingleFile>(logical, 0);
                auto decode = plan().emplace<physical::GPUDecode>(logical, scan);
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

            auto physical_parent = physical_parents[physical_parents.size() - 1];

            if(!plan().has_physical_assignment(node)) {
                plan().emplace<physical::GPUEncode>(plan().lookup(node), physical_parent);
                return true;
            }
            return false;
        }
    };

    class ChooseUnion : public OptimizerRule {
    public:
        using OptimizerRule::OptimizerRule;

        bool visit(const logical::CompositeLightField &node) override {
            auto physical_parents = functional::flatmap<std::vector<PhysicalLightFieldReference>>(
                    node.parents().begin(), node.parents().end(),
                    [this](auto &parent) { return plan().assignments(parent); });

            if(!plan().has_physical_assignment(node)) {
                plan().emplace<physical::GPUUnion>(plan().lookup(node), physical_parents);
                return true;
            }
            return false;
        }
    };
}

#endif //LIGHTDB_RULES_H
