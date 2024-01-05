#include "SimpleUndulatorSource.h"

#include "Data/xml.h"
#include "Debug/Debug.h"
#include "Debug/Instrumentor.h"
#include "Random.h"
#include "Shader/Constants.h"

namespace RAYX {

SimpleUndulatorSource::SimpleUndulatorSource(const DesignObject& dobj) : LightSource(dobj) {
    m_sigmaType = dobj.parseSigmaType();
    m_photonEnergy = dobj.parsePhotonEnergy();
    m_photonWaveLength = calcPhotonWavelength(m_photonEnergy);
    m_undulatorLength = dobj.parseUndulatorLength();
    m_undulatorSigma = calcUndulatorSigma();
    m_undulatorSigmaS = calcUndulatorSigmaS();
    m_electronSigmaX = dobj.parseElectronSigmaX();
    m_electronSigmaXs = dobj.parseElectronSigmaXs();
    m_electronSigmaY = dobj.parseElectronSigmaY();
    m_electronSigmaYs = dobj.parseElectronSigmaYs();

    

    m_sourceDepth = dobj.parseSourceDepth();
    m_sourceHeight = getSourceHeight();
    m_sourceWidth = getSourceWidth();

    m_linearPol_0 = dobj.parseLinearPol0();
    m_linearPol_45 = dobj.parseLinearPol45();
    m_circularPol = dobj.parseCircularPol();

    
}

/**
 * get deviation from main ray according to specified distribution (uniform if
 * hard edge, gaussian if soft edge)) and extent (eg specified width/height of
 * source)
 */
double SimpleUndulatorSource::getCoord(const SourceDist l, const double extent) const {
    if (l == SourceDist::Uniform) {
        return (randomDouble() - 0.5) * extent;
    } else {
        return randomNormal(0, 1) * extent;
    }
}

/**
 * Creates random rays from simple undulator Source
 *
 * @returns list of rays
 */
std::vector<Ray> SimpleUndulatorSource::getRays([[maybe_unused]] int thread_count) const {
    RAYX_PROFILE_FUNCTION();

    double x, y, z, psi, phi, en;  // x,y,z pos, psi,phi direction cosines, en=energy

    int n = m_numberOfRays;
    std::vector<Ray> rayList;
    rayList.reserve(m_numberOfRays);
    RAYX_VERB << "Create " << n << " rays with standard normal deviation...";

    // create n rays with random position and divergence within the given span
    // for width, height, depth, horizontal and vertical divergence
    for (int i = 0; i < n; i++) {
        x = getCoord(SourceDist::Gaussian, m_sourceWidth);
        y = getCoord(SourceDist::Gaussian, m_sourceHeight);
        z = (randomDouble() - 0.5) * m_sourceDepth;
        z += m_position.z;
        en = selectEnergy();  // LightSource.cpp
        glm::dvec3 position = glm::dvec3(x, y, z);

        phi = getCoord(SourceDist::Gaussian, getHorDivergence());
        psi = getCoord(SourceDist::Gaussian, getVerDivergence());
        // get corresponding angles based on distribution and deviation from
        // main ray (main ray: xDir=0,yDir=0,zDir=1 for phi=psi=0)
        glm::dvec3 direction = getDirectionFromAngles(phi, psi);
        glm::dvec4 tempDir = m_orientation * glm::dvec4(direction, 0.0);
        direction = glm::dvec3(tempDir.x, tempDir.y, tempDir.z);
        glm::dvec4 stokes = glm::dvec4(1, m_linearPol_0, m_linearPol_45, m_circularPol);

        Ray r = {position, ETYPE_UNINIT, direction, en, stokes, 0.0, 0.0, -1.0, -1.0};

        rayList.push_back(r);
    }
    return rayList;
}

double SimpleUndulatorSource::calcUndulatorSigma() const {
    double undulatorSigma;
    if (m_sigmaType == SigmaType::ST_STANDARD) {
        undulatorSigma = sqrt(2 * m_photonWaveLength / 1000 * m_undulatorLength * 1000000) / (2 * PI);  // in µm
    } else if (m_sigmaType == SigmaType::ST_ACCURATE) {
        undulatorSigma = 3.0 / (4 * PI) * sqrt(m_photonWaveLength / 1000 * m_undulatorLength * 1000000);  // in µm
    } else {
        undulatorSigma = 0;
    }
    return undulatorSigma;
}

double SimpleUndulatorSource::calcUndulatorSigmaS() const {
    double undulatorSigmaS;
    if (m_sigmaType == SigmaType::ST_STANDARD) {
        undulatorSigmaS = sqrt(m_photonWaveLength * 1000 / (2 * m_undulatorLength));  // in µrad
    } else if (m_sigmaType == SigmaType::ST_ACCURATE) {
        undulatorSigmaS = 0.53 * sqrt(m_photonWaveLength * 1000 / m_undulatorLength);  // in µrad
    } else {
        undulatorSigmaS = 0;
    }
    return undulatorSigmaS;
}

double SimpleUndulatorSource::getSourceHeight() const {
    double height;
    if (m_sigmaType == SigmaType::ST_MAINBEAM) {
        height = 0;
    } else {
        height = sqrt(pow(m_electronSigmaY, 2) + pow(m_undulatorSigma, 2));  // in µm
    }
    return height/1000;
}

double SimpleUndulatorSource::getSourceWidth() const {
    double width;
    if (m_sigmaType == SigmaType::ST_MAINBEAM) {
        width = 0;
    } else {
        width = sqrt(pow(m_electronSigmaX, 2) + pow(m_undulatorSigma, 2));  // in µm
    }
    return width/1000;
}

double SimpleUndulatorSource::getHorDivergence() const{
    double hordiv;
    if (m_sigmaType == SigmaType::ST_MAINBEAM) {
        hordiv =0;
    } else {
        hordiv = sqrt(pow(m_electronSigmaXs,2)+pow(m_undulatorSigmaS,2));  // in µrad
    }
    return hordiv;
}

double SimpleUndulatorSource::getVerDivergence() const{
    double verdiv;
    if (m_sigmaType == SigmaType::ST_MAINBEAM) {
        verdiv =0;
    } else {
        verdiv = sqrt(pow(m_electronSigmaYs,2)+pow(m_undulatorSigmaS,2));  // in µrad
    }
    return verdiv;
}

}  // namespace RAYX
