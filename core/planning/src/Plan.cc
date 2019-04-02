#include "Plan.h"
#include "PhysicalOperators.h"

namespace lightdb::optimization {
    const PhysicalOperatorReference& Plan::add(const PhysicalOperatorReference &physical) {
        physical_.insert(physical);
        assign(physical->logical(), physical);
        return physical;
    }

    void Plan::assign(const LightFieldReference &node, const PhysicalOperatorReference &physical) {
        if(assigned_.find(&*node) == assigned_.end())
            assigned_[&*node] = {};
        assigned_[&*node].insert(physical);
    }

    const set<PhysicalOperatorReference> Plan::assignments(const LightFieldReference &reference) const {
        auto element = assigned_.find(&*reference);
        return element != assigned_.end()
               ? element->second
               : set<PhysicalOperatorReference>{};
    }

    set<PhysicalOperatorReference> Plan::assignments(const LightFieldReference &reference) {
        auto element = assigned_.find(&*reference);
        return element != assigned_.end()
               ? element->second
               : set<PhysicalOperatorReference>{};
    }

    std::vector<PhysicalOperatorReference> Plan::unassigned(const LightFieldReference &reference, const bool global) const {
        auto assignments = this->assignments(reference);
        std::set<PhysicalOperator*> nonleafs;
        std::vector<PhysicalOperatorReference> leafs;

        for(const auto &assignment: assignments)
            for(auto& parent: assignment->parents())
                nonleafs.insert(&*parent);

        for(const auto &assignment: assignments)
            if(nonleafs.find(&*assignment) == nonleafs.end())
                leafs.push_back(assignment);

        //TODO this is like O(n^999) :-/
        if(global)
            for (const auto &child: children(reference))
                for (const auto &assignment: this->assignments(child))
                    for (const auto &parent: assignment->parents())
                        for (auto index = 0u; index < leafs.size(); index++)
                            if (parent == leafs[index])
                                leafs.erase(leafs.begin() + index--);
        return leafs;
    }

    set<PhysicalOperatorReference> Plan::children(const PhysicalOperatorReference &reference) const {
        set<PhysicalOperatorReference> children;

        for(const auto &node: physical_)
            for(const auto &parent: node->parents())
                if(parent == reference)
                    children.insert(node);

        return children;
    }

    void Plan::replace_assignments(const PhysicalOperatorReference &original, const PhysicalOperatorReference &replacement) {
        for(auto &[logical, assignments]: assigned_)
            if(assignments.find(original) != assignments.end()) {
                assignments.erase(original);
                assignments.insert(replacement);
            }

        for(auto &physical: physical_) {
            auto &parents = physical->parents();
            if(std::find(parents.begin(), parents.end(), original) != parents.end()) {
                parents.erase(std::remove(parents.begin(), parents.end(), replacement), parents.end());
                std::replace(parents.begin(), parents.end(), original, replacement);
            }
        }

        physical_.erase(original);
    }
} // namespace lightdb::optimization

