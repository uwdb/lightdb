#ifndef LIGHTDB_VISITOR_H
#define LIGHTDB_VISITOR_H

namespace lightdb {
    class LightFieldReference;
    class LightField;

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
    }

    class LightFieldVisitor {
    public:
        //template<typename T>
        //void visit(const LightFieldReference &lightField) { visit(static_cast<T&>(*lightField)); }

        //template<typename T>
        //virtual void visit(const LightFieldReference reference) {}
/*        virtual void visit(const logical::ConstantLightField &) {}
        virtual void visit(const logical::CompositeLightField &) {}
        virtual void visit(const logical::PartitionedLightField &) {}
        virtual void visit(const logical::SubsetLightField &) {}
        virtual void visit(const logical::RotatedLightField &) {}
        virtual void visit(const logical::DiscreteLightField &) {}
        virtual void visit(const logical::InterpolatedLightField &) {}
        virtual void visit(const logical::TransformedLightField &) {}
        virtual void visit(const logical::ScannedLightField &) {}
        virtual void visit(const logical::ExternalLightField &) {}*/

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

    protected:
        LightFieldVisitor() = default;
    };

}; // namespace lightdb

#endif //LIGHTDB_VISITOR_H
