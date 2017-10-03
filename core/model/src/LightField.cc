#include "LightField.h"

const AngularRange AngularRange::ThetaMax{0, M_2_PI};
const AngularRange AngularRange::PhiMax{0, M_PI};
const Volume Volume::VolumeMax{{0, 999}, {0, 999}, {0, 999}, {0, 999}, AngularRange::ThetaMax, AngularRange::PhiMax};
const YUVColorSpace YUVColorSpace::Instance{};
const YUVColor YUVColor::Green{149, 43, 21};
const YUVColor YUVColor::Null{255, 255, 255};
const EquirectangularGeometry EquirectangularGeometry::Instance;


