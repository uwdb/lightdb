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
        class SubqueriedLightField;
        class ExternalLightField;
        class EncodedLightField;
        class StoredLightField;
        class SunkLightField;
    }

    class LightFieldVisitor {
    public:
        virtual void visit(const LightField &) {}
        virtual void visit(const logical::ConstantLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::CompositeLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::PartitionedLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::SubsetLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::RotatedLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::DiscreteLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::InterpolatedLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::TransformedLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::ScannedLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::SubqueriedLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::ExternalLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::EncodedLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::StoredLightField &l) { visit((const LightField&)l); }
        virtual void visit(const logical::SunkLightField &l) { visit((const LightField&)l); }

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
        virtual State visit(const logical::ConstantLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::CompositeLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::PartitionedLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::SubsetLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::RotatedLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::DiscreteLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::InterpolatedLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::TransformedLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::ScannedLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::SubqueriedLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::ExternalLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::EncodedLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::StoredLightField &l) { return visit((const LightField&)(l)); }
        virtual State visit(const logical::SunkLightField &l) { return visit((const LightField&)(l)); }

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
        virtual State visit(State state, const logical::SubqueriedLightField &field) {return state + visit(field); }
        virtual State visit(State state, const logical::ExternalLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::EncodedLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::StoredLightField &field) { return state + visit(field); }
        virtual State visit(State state, const logical::SunkLightField &field) { return state + visit(field); }

        class AdaptedVisitor: public LightFieldVisitor {
        public:
            AdaptedVisitor() = default;
            explicit AdaptedVisitor(StatefulLightFieldVisitor &visitor, State initial_state)
                    : visitor_(visitor), state_(std::move(initial_state)) { }

            void visit(const LightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::ConstantLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::CompositeLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::PartitionedLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::SubsetLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::RotatedLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::DiscreteLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::InterpolatedLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::TransformedLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::ScannedLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::SubqueriedLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::ExternalLightField &field) final { state_ = visitor_.visit(state_, field); }
            void visit(const logical::EncodedLightField &field) final  { state_ = visitor_.visit(state_, field); }
            void visit(const logical::StoredLightField &field) final  { state_ = visitor_.visit(state_, field); }
            void visit(const logical::SunkLightField &field) final  { state_ = visitor_.visit(state_, field); }

            State result() const { return state_; }

        private:
            StatefulLightFieldVisitor &visitor_;
            State state_;
        };
    };

}; // namespace lightdb

#endif //LIGHTDB_VISITOR_H
