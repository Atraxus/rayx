#pragma once

#include "Common.h"
#include "Ray.h"
#include "InvocationState.h"

namespace RAYX {

RAYX_FUNC void init(Inv& inv);
RAYX_FUNC uint64_t rayId(Inv& inv);
RAYX_FUNC uint output_index(uint i, Inv& inv);
RAYX_FUNC void recordEvent(Ray r, double w, Inv& inv);
RAYX_FUNC void recordFinalEvent(Ray r, double w, Inv& inv);

} // namespace RAYX
