#pragma once
#include "Model/Beamline/OpticalElement.h"
#include "Model/Surface/Quadric.h"

namespace RAYX {

class RAYX_API ImagePlane : public OpticalElement {
  public:
    ImagePlane(const char* name, glm::dvec4 position, glm::dmat4x4 orientation);
    ImagePlane();
    ~ImagePlane();

    static std::shared_ptr<ImagePlane> createFromXML(rapidxml::xml_node<>*);

    double getDistance();
};
}  // namespace RAYX
