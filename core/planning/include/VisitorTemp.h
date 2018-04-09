#ifndef LIGHTDB_PHYSICAL_H
#define LIGHTDB_PHYSICAL_H

#include "LightField.h"
#include "Encoding.h"
#include "Functor.h"
#include "TileVideoEncoder.h" //TODO can remove this after hacks addressed
#include <tuple>
#include <stdexcept>

namespace lightdb::planning {
    class PlanDispatcher;

    /*
    class PlanNode {
    public:
        explicit PlanNode(const LightFieldReference logical): logical_(logical) { }

        const LightFieldReference logical() const noexcept { return logical_; }

        void accept(PlanDispatcher &dispatcher);

    private:
        const LightFieldReference &logical_;
    };
     */

    class Plan {
    public:
        explicit Plan(const LightFieldReference &root): root_(root) { }

    private:
        const LightFieldReference root_;
    };

    template<typename T>
    class PlanNode {
    public:
        explicit PlanNode(const LightFieldReference &logical): logical_(logical) { }

        const T& logical() const noexcept { return *logical_; }

        void accept(PlanDispatcher &dispatcher);

    private:
        const LightFieldReference logical_;
    };

    class LogicalDispatcher {
    public:
        virtual void dispatch(const LightFieldReference reference, const logical::ScannedLightField &lightField) { }
        virtual void dispatch(const LightFieldReference reference, const logical::CompositeLightField &lightField) { }
    };


    class PlanDispatcher {
    public:
        template<typename T>
        static PlanNode<T> dispatch(LightFieldReference reference) {
            return PlanNode<T>(reference);
        }

        virtual void dispatch(PlanNode<logical::ScannedLightField> node) { }
        virtual void dispatch(PlanNode<logical::CompositeLightField> node) { }
    };

    template PlanNode<logical::ScannedLightField> PlanDispatcher::dispatch<logical::ScannedLightField>(LightFieldReference);

    template<typename T>
    void PlanNode<T>::accept(PlanDispatcher &dispatcher) {
        dispatcher.dispatch(*this);
    }

    class PlanInitializer: public LogicalDispatcher {
    public:
        void dispatch(const LightFieldReference reference, const logical::ScannedLightField &lightField) override {
            printf("ScannedLightField\n");
            //for(const LightFieldReference &p: reference->parents())
              //  dispatch(p, *p);
        }
        void dispatch(const LightFieldReference reference, const logical::CompositeLightField &lightField) override {
            printf("CompositeLightField\n");
        }
    };

    class ChooseDecoders: public PlanDispatcher {
    public:
        void dispatch(PlanNode<logical::ScannedLightField> node) override {
            printf("ScannedLightField\n");
        }
        void dispatch(PlanNode<logical::CompositeLightField> node) override {
            printf("CompositeLightField\n");
        }
    };


} // namespace lightdb

#endif //LIGHTDB_PHYSICAL_H
