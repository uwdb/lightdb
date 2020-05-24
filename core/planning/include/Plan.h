#ifndef LIGHTDB_PLAN_H
#define LIGHTDB_PLAN_H

#include "LightField.h"
#include "Environment.h"
#include "Allocator.h"
#include "set.h"

namespace lightdb {
    class PhysicalOperator;
    using PhysicalOperatorReference = shared_reference<PhysicalOperator, AddressableMixin<PhysicalOperator>>;
}

namespace lightdb::optimization {
    class Plan {
    public:
        explicit Plan(const execution::Environment &environment, const LightFieldReference &sink)
                : Plan(environment, std::vector{sink}) {}

        explicit Plan(const execution::Environment &environment, const std::vector<LightFieldReference> &sinks)
                : Plan(environment, sinks.begin(), sinks.end()) { }

        template<typename InputIterator>
        explicit Plan(execution::Environment environment, const InputIterator first, const InputIterator last)
                : environment_(std::move(environment)),
                  allocator_(AllocatorReference::make<RoundRobinAllocator>(environment_)),
                  sinks_(first, last),
                  physical_() {
            std::for_each(first, last, std::bind(&Plan::associate, this, std::placeholders::_1));
        }

        Plan(const Plan&) = default;
        Plan(Plan&&) = default;

        void associate(const LightFieldReference &node) {
            nodes_.insert(std::make_pair(node.operator->(), node));
            std::for_each(node->parents().begin(), node->parents().end(),
                          std::bind(&Plan::associate, this, std::placeholders::_1));
        }

        const PhysicalOperatorReference& add(const PhysicalOperatorReference &physical);

        template<typename PhysicalLightField, typename... Args>
        const PhysicalOperatorReference& emplace(Args&&... args) {
            const auto &value = *physical_.emplace(PhysicalOperatorReference::make<PhysicalLightField>(args...)).first;
            assign(value->logical(), value);
            return value;
        }

        void assign(const LightFieldReference &node, const PhysicalOperatorReference &physical);

        inline bool has_physical_assignment(const LightField &node) { return has_physical_assignment(lookup(node)); }

        bool has_physical_assignment(const LightFieldReference &reference) {
            return assigned_.find(&*reference) != assigned_.end();
        }

        inline auto assignments(const LightField &node) const { return assignments(lookup(node)); }

        const set<PhysicalOperatorReference> assignments(const LightFieldReference &reference) const;

        set<PhysicalOperatorReference> assignments(const LightFieldReference &reference);

        std::vector<PhysicalOperatorReference> unassigned(const LightFieldReference &reference, bool global=true) const;

        std::vector<LightFieldReference> children(const LightFieldReference &reference) const {
            std::vector<LightFieldReference> children;

            for(const auto &node: nodes_)
                for(const LightFieldReference &parent: node.second->parents())
                    if(parent == reference)
                        children.push_back(node.second);

            return children;
        }

        set<PhysicalOperatorReference> children(const PhysicalOperatorReference &reference) const;

        void replace_assignments(const PhysicalOperatorReference &original, const PhysicalOperatorReference &replacement);

        inline auto& physical() { return physical_; }
        inline const auto& physical() const { return physical_; }
        inline const auto& environment() const { return environment_; }
        inline auto& allocator() const { return *allocator_; }
        inline LightFieldReference lookup(const LightField &node) const { return nodes_.at(&node); }
        inline const std::vector<LightFieldReference>& sinks() const { return sinks_; }

    private:
        const execution::Environment environment_;
        AllocatorReference allocator_;
        std::vector<LightFieldReference> sinks_;
        //TODO can be replaced with an addressable shared_reference
        std::unordered_map<const LightField *, LightFieldReference> nodes_;
        std::unordered_map<const LightField*, set<PhysicalOperatorReference>> assigned_;
        set<PhysicalOperatorReference> physical_;

    };
}

#endif //LIGHTDB_PLAN_H
