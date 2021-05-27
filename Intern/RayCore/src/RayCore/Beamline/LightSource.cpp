#include "LightSource.h"
#include <cassert>
#include <cmath>

namespace RAY
{
    LightSource::LightSource(int id, int numberOfRays, const char* name, std::vector<double> misalignment) : m_id(id), m_numberOfRays(numberOfRays), m_name(name), m_misalignmentParams(misalignment){}

    const char* LightSource::getName() { return m_name; }
    int LightSource::getNumberOfRays() { return m_numberOfRays; }
    void LightSource::setNumberOfRays(int numberOfRays) {m_numberOfRays = numberOfRays; }
    int LightSource::getId() { return m_id; }
    std::vector<double> LightSource::getMisalignmentParams() { return m_misalignmentParams; }
    
    // needed for many of the light sources
    glm::dvec3 LightSource::getDirectionFromAngles(double phi, double psi){
        double al = cos(psi)*sin(phi);
        double am = -sin(psi);
        double an = cos(psi)*cos(phi);
        return glm::dvec3(al,am,an);
    }

    LightSource::~LightSource()
    {
    }

} // namespace RAY