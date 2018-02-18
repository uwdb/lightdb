#include "LightField.h"

namespace lightdb {

LightField::LightField(const LightFieldReference &parent)
    : LightField({parent}, parent->volume(), parent->colorSpace())
{ }

LightField::LightField(const LightFieldReference &parent, const CompositeVolume &volume)
    : LightField(std::vector<LightFieldReference>{parent}, volume, parent->colorSpace())
{ }


LightField::LightField(std::vector<LightFieldReference> &parents, const ColorSpace &colorSpace)
    : LightField(parents,
                 CompositeVolume{functional::transform<Volume>(parents.begin(), parents.end(), [](auto &parent) {
                                     return parent->volume().bounding(); })},
                 colorSpace)
{ }

LightField::LightField(const LightFieldReference &parent, const CompositeVolume &volume, const ColorSpace &colorSpace)
    : LightField(std::vector<LightFieldReference>{parent}, volume, colorSpace)
{ }

} // namespace lightdb::logical