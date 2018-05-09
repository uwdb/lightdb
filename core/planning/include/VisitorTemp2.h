#ifndef LIGHTDB_VISITORTEMP2_H
#define LIGHTDB_VISITORTEMP2_H

#include "LightField.h"
#include "Physical.h"
#include "Plan.h"

namespace lightdb::optimization {
/*
    class PlanDispatcher;

    class PlanReferenceNode {
    public:
        explicit PlanReferenceNode(const LightFieldReference reference): reference_(reference) { }

        const LightFieldReference reference() const noexcept { return reference_; }

        bool operator<(const PlanReferenceNode &other) const { return reference_.operator->() < other.reference_.operator->(); }

    private:
        const LightFieldReference &reference_;
    };

    template<typename T>
    class PlanNode: public PlanReferenceNode {
    public:
        explicit PlanNode(const LightFieldReference &logical)
                : PlanReferenceNode(logical), logical_(logical->downcast<T>())
        { }

        const T& logical() const noexcept { return logical_; }

        void accept(PlanDispatcher &dispatcher);

    private:
        const T& logical_;
    };

    class PlanDispatcher {
    public:
        template<typename T>
        static PlanNode<T> dispatch(LightFieldReference reference) {
            return PlanNode<T>(reference);
        }

        virtual void dispatch(PlanNode<logical::ScannedLightField>) {}

        virtual void dispatch(PlanNode<logical::CompositeLightField>) {}
    };
*/
    /*
    class Plan {
    public:
        Plan() = default;
        explicit Plan(const LightFieldReference &source) : Plan(std::vector{source}) { }
        explicit Plan(const std::vector<LightFieldReference> &sources) : Plan(sources.begin(), sources.end()) { }
            template<typename InputIterator>
        explicit Plan(const InputIterator first, const InputIterator last)
            { std::for_each(first, last, std::bind(&Plan::associate, this, std::placeholders::_1)); }

        void associate(const LightFieldReference& node)
        {
            nodes_.insert(std::make_pair(node.operator->(), node));
            std::for_each(node->parents().begin(), node->parents().end(),
                          std::bind(&Plan::associate, this, std::placeholders::_1));
        }

        void add(const PhysicalLightFieldReference& physical)
        {
            physical_.insert(physical);
        }


        LightFieldReference lookup(const LightField& node) { return nodes_.at(&node); }

    private:
        //constexpr static auto physical_compare = [](const auto& left, const auto& right) { return false; };
        //static bool physical_compare(const auto& left, const auto& right)
        //    { return left.operator->() < right.operator->(); }

        std::unordered_map<const LightField*, LightFieldReference> nodes_;
        std::set<PhysicalLightFieldReference,
                bool(*)(const PhysicalLightFieldReference&,const PhysicalLightFieldReference&)> physical_;

    };
*/

    /*
    class ChooseDecoders2: public LightFieldVisitor {
    public:
        explicit ChooseDecoders2(Plan &plan) : plan_(plan) { }

        void visit(const logical::ScannedLightField& node) override {
            plan_.add(physical::GPUDecode(plan_.lookup(node)));
        }

        void visit(const logical::CompositeLightField& node) override {
            auto n = plan_.lookup(node);
            printf("#CompositeLightField\n");
            printf("%d\n", n.operator->() == &node);
            //plan_.add(PlanNode<logical::ScannedLightField>(node));
        }

    private:
        Plan &plan_;
    };
     */

}; // namespace lightdb::planning
#endif //LIGHTDB_VISITORTEMP2_H
