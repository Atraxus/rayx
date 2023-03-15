#pragma once

#include <array>
#include <glm.hpp>

#include "Constants.h"
#include "Core.h"
#include "Data/xml.h"
#include "utils.h"

namespace RAYX {

enum class GratingMount { Deviation, Incidence };
enum class ImageType { Point2Point, Astigmatic2Astigmatic };

class RAYX_API OpticalElement {
  public:
    struct Geometry {
        Rad m_azimuthalAngle = Rad(0);                // rotation of element through xy-plane
                                                      // (needed for stokes vector)
        glm::dmat4x4 m_orientation = glm::dmat4x4();  //< Orientation matrix of element (is basis)
        glm::dvec4 m_position = glm::dvec4();         //< Position of element in world coordinates

        Geometry();
        Geometry(const Geometry& other);
    };

    OpticalElement(const DesignObject&);
    // needed to add optical elements to tracer
    OpticalElement(const char* name, const std::array<double, 7> slopeError,
                   const Geometry& geometry = Geometry());  // TODO(Jannis): add surface

    /// Converts `this` into an Element.
    Element intoElement() const;

    virtual ~OpticalElement() = default;

    void calcTransformationMatrices(glm::dvec4 position, glm::dmat4 orientation, glm::dmat4& output, bool calcInMatrix = true) const;

    glm::dmat4 getInMatrix() const;
    glm::dmat4 getOutMatrix() const;
    glm::dmat4x4 getOrientation() const;
    glm::dvec4 getPosition() const;

    virtual std::array<double, 16> getElementParams() const;
    virtual int getElementType() const = 0;
    std::array<double, 7> getSlopeError() const;

    std::string m_name;
    Material m_material;

  protected:
    std::unique_ptr<Geometry> m_Geometry;  ///< Geometry of the element

    int m_surfaceType;
    std::array<double, 16> m_surfaceParams;

    std::array<double, 7> m_slopeError;
    int m_excerptType;
    std::array<double, 3> m_excerptParams;
};

}  // namespace RAYX
