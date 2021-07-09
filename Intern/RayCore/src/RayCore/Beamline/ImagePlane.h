#pragma once
#include "Surface/Quadric.h"
#include "OpticalElement.h"

namespace RAY
{

    class RAY_API ImagePlane : public OpticalElement {

    public:

        ImagePlane(const char* name, const double distance, const std::shared_ptr<OpticalElement> previous);
        ImagePlane();
        ~ImagePlane();

        double getDistance();

    private:
        double m_distance;
    };
}
