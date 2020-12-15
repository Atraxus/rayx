#pragma once

#include "Beamline/Beamline.h"

#include "Core.h"
#include "Ray.h"

#include <vector>

namespace RAY
{
    class RAY_API TracerInterface
    {
    public:
        TracerInterface();
        ~TracerInterface();

        bool run();

    private:
        Beamline m_Beamline;
        std::vector<Ray *> m_RayList;
    };
} // namespace RAY