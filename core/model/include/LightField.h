#ifndef LIGHTDB_LIGHTFIELD_H
#define LIGHTDB_LIGHTFIELD_H

#include "Algebra.h"
#include "Geometry.h"
#include "Color.h"
#include "Visitor.h"
#include "reference.h"
#include <vector>

namespace lightdb {
    class LightField;
    using LightFieldReference = shared_reference<LightField, logical::Algebra>;

    class LightField {
    protected:
        explicit LightField(const LightFieldReference &parent);
        explicit LightField(std::vector<LightFieldReference> &parents, const ColorSpace&);
        explicit LightField(const LightFieldReference &parent, const CompositeVolume&);
        explicit LightField(const LightFieldReference &parent, const CompositeVolume&, const ColorSpace&);
        explicit LightField(std::vector<LightFieldReference> parents, CompositeVolume volume,
                            const ColorSpace& colorSpace)
            : parents_(std::move(parents)), volume_(std::move(volume)), colorSpace_(colorSpace)
        { }

        virtual ~LightField() = default;

    public:
        template<typename T>
        inline T& downcast() {
            return dynamic_cast<T&>(*this);
        }

        template<typename T>
        inline const T& downcast() const {
            return dynamic_cast<const T&>(*this);
        }

        template<typename T>
        inline bool is() const {
            return dynamic_cast<const T*>(this) != nullptr;
        }

        template<typename T>
        inline const std::optional<std::reference_wrapper<const T>> try_downcast() const {
            return {dynamic_cast<const T&>(*this)};
        }

        virtual inline const std::vector<LightFieldReference>& parents() const noexcept { return parents_; }

        virtual inline const ColorSpace colorSpace() const noexcept { return colorSpace_; }

        virtual inline const CompositeVolume& volume() const noexcept { return volume_; }

        inline std::string type() const noexcept { return typeid(*this).name(); }

        virtual void accept(LightFieldVisitor &visitor) = 0;

    protected:
        //TODO think we should have a LightField<T>:LightField that does this all automatically
        //TODO same for LightFieldReference, then we can downcast without a dynamic call
        template<typename T>
        void accept(LightFieldVisitor &visitor) {
            visitor.visit(static_cast<T&>(*this));
            std::for_each(parents().begin(), parents().end(), [&visitor](auto parent) {
                parent->accept(visitor); });
        }

    private:
        const std::vector<LightFieldReference> parents_;
        const CompositeVolume volume_;
        const ColorSpace colorSpace_;
    };
} // namespace lightdb
#endif //LIGHTDB_LIGHTFIELD_H
