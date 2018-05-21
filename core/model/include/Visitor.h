#ifndef LIGHTDB_VISITOR_H
#define LIGHTDB_VISITOR_H

namespace lightdb {
    class LightField;
    namespace logical { class Algebra; }
    using LightFieldReference = shared_reference<LightField, logical::Algebra>;

    namespace logical {
        class ConstantLightField;
        class CompositeLightField;
        class PartitionedLightField;
        class SubsetLightField;
        class RotatedLightField;
        class DiscreteLightField;
        class InterpolatedLightField;
        class TransformedLightField;
        class ScannedLightField;
        class ExternalLightField;
        class EncodedLightField;
    }

    class LightFieldVisitor {
    public:
        virtual void visit(const LightField &) {}
        virtual void visit(const logical::ConstantLightField &) {}
        virtual void visit(const logical::CompositeLightField &) {}
        virtual void visit(const logical::PartitionedLightField &) {}
        virtual void visit(const logical::SubsetLightField &) {}
        virtual void visit(const logical::RotatedLightField &) {}
        virtual void visit(const logical::DiscreteLightField &) {}
        virtual void visit(const logical::InterpolatedLightField &) {}
        virtual void visit(const logical::TransformedLightField &) {}
        virtual void visit(const logical::ScannedLightField &) {}
        virtual void visit(const logical::ExternalLightField &) {}
        virtual void visit(const logical::EncodedLightField &) {}

    protected:
        LightFieldVisitor() = default;
    };

    namespace internal {
        // Avoid templated circular dependency between LightFieldReference and StatefulLightFieldVisitor
        class VisitorDelegator {
        public:
            static void accept_delegate(LightFieldVisitor&, const LightFieldReference&);
        };
    }

    template<typename State>
    class StatefulLightFieldVisitor {
    public:
        State accept(const LightFieldReference &acceptor) {
            return accept(acceptor, {});
        }
        State accept(const LightFieldReference &acceptor, State initial_state) {
            AdaptedVisitor visitor(*this, initial_state);
            internal::VisitorDelegator::accept_delegate(visitor, acceptor);
            return visitor.result();
        }


        virtual State visit(const LightField &) { return {}; }
        virtual State visit(const logical::ConstantLightField &) { return {}; }
        virtual State visit(const logical::CompositeLightField &) { return {}; }
        virtual State visit(const logical::PartitionedLightField &) { return {}; }
        virtual State visit(const logical::SubsetLightField &) { return {}; }
        virtual State visit(const logical::RotatedLightField &) { return {}; }
        virtual State visit(const logical::DiscreteLightField &) { return {}; }
        virtual State visit(const logical::InterpolatedLightField &) { return {}; }
        virtual State visit(const logical::TransformedLightField &) { return {}; }
        virtual State visit(const logical::ScannedLightField &) { return {}; }
        virtual State visit(const logical::ExternalLightField &) { return {}; }
        virtual State visit(const logical::EncodedLightField &) { return {}; }

        virtual State visit(State state, const LightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::ConstantLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::CompositeLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::PartitionedLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::SubsetLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::RotatedLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::DiscreteLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::InterpolatedLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::TransformedLightField &field) {return state + visit(field); }
        virtual State visit(State state, const logical::ScannedLightField &field) {return state + visit(field); }
        virtual State visit(State state, const logical::ExternalLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::EncodedLightField &field) { return state + visit(field); }

        class AdaptedVisitor: public LightFieldVisitor {
        public:
            AdaptedVisitor() = default;
            explicit AdaptedVisitor(StatefulLightFieldVisitor &visitor, State initial_state)
                    : visitor_(visitor), state_(std::move(initial_state)) { }

            void visit(const LightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::ConstantLightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::CompositeLightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::PartitionedLightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::SubsetLightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::RotatedLightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::DiscreteLightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::InterpolatedLightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::TransformedLightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::ScannedLightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::ExternalLightField &field) final { visitor_.visit(state_, field); }
            void visit(const logical::EncodedLightField &field) final  { visitor_.visit(state_, field); }

            State result() const { return state_; }

        private:
            StatefulLightFieldVisitor &visitor_;
            State state_;
        };
    };

}; // namespace lightdb

#endif //LIGHTDB_VISITOR_H
