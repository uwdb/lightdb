#ifndef LIGHTDB_VISITORTEMP2_H
#define LIGHTDB_VISITORTEMP2_H

#include "LightField.h"
#include <set>

namespace lightdb::planning {
    class PlanDispatcher;

    class PlanReferenceNode {
    public:
        explicit PlanReferenceNode(const LightFieldReference reference): reference_(reference) { }

        const LightFieldReference reference() const noexcept { return reference_; }

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

    class Plan {
    public:
        void add(PlanReferenceNode&& node) {
            //nodes.insert(node);
        }

    private:
        std::set<PlanReferenceNode> nodes;
    };

    class ChooseDecoders: public LightFieldVisitor {
    public:
        explicit ChooseDecoders(Plan &plan) : plan_(plan) { }

        void visit(const logical::ScannedLightField& node) override {
            //plan_.add(PlanNode<logical::ScannedLightField>(node));
        }
        void visit(const logical::CompositeLightField& node) override {
            printf("CompositeLightField\n");
        }

    private:
        Plan &plan_;
    };
}; // namespace lightdb::planning
#endif //LIGHTDB_VISITORTEMP2_H
