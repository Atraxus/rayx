#include "DipoleSource.h"

#include "Data/xml.h"
#include "Debug/Debug.h"
#include "Random.h"
#include "Shared/Constants.h"
//#include "Shared/TranslatedFortran.h"

namespace RAYX {

double get_factorCriticalEnergy() {
    // double planc = Planck_bar / (2*pow(c_elementaryCharge, 5) * pow(c_electronMass,3));
    //  3 * planc * pow(c_electronVolt,2) * 1.0e24; //nach RAY-UI
    return 2.5050652873563215;
}

double get_factorMagneticField() { return c_electronVolt / (c_speedOfLight * c_elementaryCharge) * 1.0e9; }

double get_factorElectronEnergy() {
    double factorElectronEnergy = c_electronVolt * 1.0e9 / (c_electronMass * pow(c_speedOfLight, 2));
    return factorElectronEnergy;
}

double get_factorOmega() {
    return 3 * c_alpha / (4.0 * pow(PI, 2) * c_elementaryCharge * pow(c_speedOfLight, 4) * pow(c_electronMass, 2) / pow(c_electronVolt * 1.0e9, 2));
}

double get_factorDistribution() { return 3 * c_alpha / (4.0 * pow(PI, 2) * c_elementaryCharge); }

double get_factorTotalPowerDipol() {
    return pow(c_elementaryCharge, 2) / (3 * c_electricPermittivity * pow(c_speedOfLight, 8) * pow(c_electronMass, 4)) *
           pow(c_electronVolt * 1.0E9, 3) / (2 * PI) / (c_electronVolt / (c_speedOfLight * c_elementaryCharge));
}

DipoleSource::DipoleSource(const DesignObject& dobj) : LightSource(dobj) {
    m_energySpreadType = dobj.parseEnergyDistribution();
    m_photonFlux = dobj.parsePhotonFlux();
    m_electronEnergyOrientation = dobj.parseElectronEnergyOrientation();
    m_sourcePulseType = dobj.parseSourcePulseType();
    m_bendingRadius = dobj.parseBendingRadiusDouble();
    m_electronEnergy = dobj.parseElectronEnergy();
    m_photonEnergy = dobj.parsePhotonEnergy();
    m_verEbeamDivergence = dobj.parseVerEbeamDivergence();
    m_energySpread = dobj.parseEnergySpread();
    m_energySpreadUnit = dobj.parseEnergySpreadUnit();

    m_criticalEnergy = RAYX::get_factorCriticalEnergy();
    m_bandwidth = 1.0e-3;
    m_verDivergence = DipoleSource::vDivergence(m_photonEnergy, m_verEbeamDivergence);
    m_stokes = DipoleSource::getStokesSyn(m_photonEnergy, -3 * m_verDivergence, 3 * m_verDivergence);

    m_flux = (m_stokes[2] + m_stokes[3]) * m_horDivergence * 1.0e-3 * 1.0e2;  // EnergyDistribution Values
    m_gamma = std::fabs(m_electronEnergy) * get_factorElectronEnergy();
    m_maxIntensity = getMaxIntensity();
    calcFluxOrg();
    calcHorDivDegSec();
    // calcMagneticField();
    calcPhotonWavelength();
    calcSourcePath();
    setMaxEnergy();
    setLogInterpolation();
}

/**
 * Creates random rays from dipole source with specified width and height
 * distributed according to either uniform or gaussian distribution across width
 * & height of source the deviation of the direction of each ray from the main
 * ray (0,0,1, phi=psi=0) can also be specified to be uniform or gaussian within
 * a given range (m_verDivergence, m_horDivergence) z-position of ray is always
 * from uniform distribution
 *
 * @returns list of rays
 */
std::vector<Ray> DipoleSource::getRays() const {
    double x1, x, y, z, psi, phi, en;  // x,y,z pos, psi,phi direction cosines, en=energy

    int n = m_numberOfRays;
    std::vector<Ray> rayList;
    rayList.reserve(m_numberOfRays);
    // rayList.reserve(1048576);
    RAYX_VERB << "Create " << n << " rays with standard normal deviation...";

    // create n rays with random position and divergence within the given span
    // for width, height, depth, horizontal and vertical divergence
    for (int i = 0; i < n; i++) {
        phi = (randomDouble() - 0.5) * m_horDivergence;

        x1 = randomNormal(0,1) * m_sourceWidth;
        x = x1 + m_position.x;
        // x =
        y = randomNormal(0,1) * m_sourceHeight;
        y += m_position.y;
        z = 0.0;
        z += m_position.z;

        en = DipoleSource::getEnergy();  // Verteilung nach Schwingerfunktion
        glm::dvec3 position = glm::dvec3(x, y, z);
        psi = getPsi(en);

        // get random deviation from main ray based on distribution

        // get corresponding angles based on distribution and deviation from
        // main ray (main ray: xDir=0,yDir=0,zDir=1 for phi=psi=0)
        glm::dvec3 direction = getDirectionFromAngles(phi, psi);
        glm::dvec4 tempDir = m_orientation * glm::dvec4(direction, 0.0);
        direction = glm::dvec3(tempDir.x, tempDir.y, tempDir.z);
        glm::dvec4 stokes = glm::dvec4(1, 0, m_stokes[1], m_stokes[2]);

        Ray r = {position, W_UNINIT, direction, en, stokes, 0.0, 0.0, 0.0, 0.0};

        rayList.push_back(r);
    }
    return rayList;
}

double DipoleSource::getPsi(double en) const {
    double psi;

    glm::dvec4 S;

    do {
        psi =  (randomDouble() -0.5) * 6 * m_verDivergence;
        S = dipoleFold(psi, en, m_verEbeamDivergence);

    } while ((S[2] + S[3]) / m_maxIntensity - randomDouble() < 0);

    return psi;
}

double DipoleSource::getEnergy() const {
    double flux = 0.0;
    double energy = 0.0;

    do {
        energy = m_photonEnergy + (randomDouble() - 0.5) * m_energySpread;
        flux = schwinger(energy);
    } while ((flux / m_maxFlux - randomDouble()) < 0);
    return energy;
}

double DipoleSource::vDivergence(double hv, double sigv) const {
    double gamma = fabs(m_electronEnergy) * 1957;  // factorElectronEnergy
    if (gamma == 0.0 || m_criticalEnergy == 0.0) {
        return 0;
    }
    double psi = get_factorOmega() * 1.e-18 * 0.1 / gamma * pow(m_criticalEnergy * 1000.0 / hv, 0.43);
    return sqrt(pow(psi, 2) + pow(sigv * 0.001, 2));
}

glm::dvec4 DipoleSource::getStokesSyn(double hv, double psi1, double psi2) const {
    double fak = 3453345200000000.0;  // getFactorDistribution

    double gamma = fabs(m_electronEnergy) * 1957;  // getFactorElectronEnergy
    double y0 = hv / m_criticalEnergy / 1000.0;
    double xnue1 = 1.0 / 3.0;
    double xnue2 = 2.0 / 3.0;

    double dpsi = (psi2 - psi1) / 101.0;
    double psi = psi1 + dpsi / 2.0;

    if (dpsi < 0.001) {
        dpsi = 0.001;
    }

    //std::array<double, 6> result = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    glm::dvec4 stokes = {0.0, 0.0, 0.0, 0.0};
    while (psi <= psi2) {
        double sign1 = DipoleSource::m_electronEnergyOrientation == ElectronEnergyOrientation::Clockwise ? PI : -PI;
        double sign2 = psi >= 0.0 ? 1.0 : -1.0;
        double phase = -(sign1 * sign2);
        double x = gamma * psi * 0.001;
        double zeta = pow(1.0 + pow(x, 2), (3.0 / 2.0)) * 0.5 * y0;
        double xkn2 = bessel(xnue2, zeta);
        double xkn1 = bessel(xnue1, zeta);
        double xint = fak * pow(gamma, 2.0) * pow(y0, 2.0) * pow(1.0 + pow(x, 2.0), 2.0);
        double xintp = xint * pow(xkn2, 2.0);
        double xints = xint * (pow(x, 2.0) / (1.0 + pow(x, 2.0)) * pow(xkn1, 2.0));
        xintp = xintp * dpsi * 1e-6;
        xints = xints * dpsi * 1e-6;

        stokes[0] = stokes[0] + xintp - xints;
        stokes[1] = stokes[1] + 2.0 * sqrt(xintp * xints) * sin(phase);
        stokes[2] = stokes[2] + xintp;
        stokes[3] = stokes[3] + xints;
        psi = psi + dpsi;
    }

    //result[5] = result[3] + result[4];

    return stokes;
}

double DipoleSource::bessel(double hnue, double zeta) const {
    double result;
    double h = 0.1;
    result = h / 2.0 * exp(-zeta);
    double c1 = 1;
    double c2 = 0;
    for (int i = 1; c1 > c2; i++) {
        double cosh1 = (exp(h * i) + exp(-h * i)) / 2.0;
        double cosh2 = (exp(h * i * hnue) + exp(-h * i * hnue)) / 2.0;
        if ((zeta * cosh1) > 225) {
            return result;
        }
        c1 = h * exp(-zeta * cosh1) * cosh2;
        result = result + c1;
        c2 = result / 1e6;
    }
    return result;
}

void DipoleSource::setLogInterpolation() {
    for (uint i = 0; i < m_schwingerX.size(); i++) {
        m_schwingerY[i] = m_schwingerX[i] * m_schwingerY[i];
        m_schwingerX[i] = log(m_schwingerX[i]);
        m_schwingerY[i] = log(m_schwingerY[i]);
    }
}

double DipoleSource::schwinger(double energy) const {
    double vorfaktor = factorSchwinger_RAY * 1.e-3;

    double Y0 = energy / m_criticalEnergy;
    Y0 = Y0 / 1000;
    double yg0 = 0.0;
    double flux;

    if (Y0 > 0) {
        if (Y0 > 10) {
            yg0 = sqrt(PI / 2) * sqrt(energy) * pow(-energy, 2);  // sqrt(PI(T)/2.)*sqrt(z)*exp(-z)
        }
        if (Y0 < 1.e-4) {
            yg0 = 2.1495282415 * pow(energy, (1.0 / 3.0));
        } else {
            yg0 = exp(getInterpolation(log(Y0)));
        }
        // yg0 = linear oder diaboli
    }

    flux = vorfaktor * m_gamma * yg0;

    return flux;
}

double DipoleSource::getMaxIntensity() {
    double smax = 0.0;
    double psi = -m_verDivergence;

    for (int i = 1; i < 250; i++) {
        psi = psi + 0.05;
        auto S = dipoleFold(psi, m_photonEnergy, 1.0);
        if (smax < S[5]) {
            smax = S[5];
        } else {
            break;
        }
    }
    return smax;
}

glm::dvec4 DipoleSource::dipoleFold(double psi, double photonEnergy, double sigpsi) const {
    int ln = (int)sigpsi;
    double trsgyp = 0.0;
    double sgyp = 0.0;
    double sy = 0.0;
    double zw = 0.0;
    double wy = 0.0;
    double psi1 = 0.0;

    glm::dvec4 ST = {0.0, 0.0, 0.0, 0.0};
    glm::dvec4 S;

    if (sigpsi != 0) {
        if (ln > 10) {
            ln = 10;
        }
        if (ln == 0) {
            ln = 10;
        }
        trsgyp = -0.5 / sigpsi / sigpsi;
        sgyp = 4.0e-3 * sigpsi;
    } else {
        trsgyp = 0;
        ln = 1;
    }

    for (int i = 1; i <= ln; i++) {
        do {
            sy = (randomDouble() - 0.5) * sgyp;
            zw = trsgyp * sy * sy;
            wy = exp(zw);
        } while (wy - randomDouble() < 0);

        psi1 = psi + sy;
        S = getStokesSyn(photonEnergy, psi1, psi1);

        for (int i = 0; i < 6; i++) {
            ST[i] = ST[i] + S[i];
        }
    }

    for (int i = 0; i < 6; i++) {
        S[i] = ST[i] / ln;
    }

    psi = psi1;

    return S;
}

void DipoleSource::calcMagneticField() {
    m_magneticFieldStrength = get_factorMagneticField() * fabs(m_electronEnergy) / m_bendingRadius;
    m_criticalEnergy = get_factorCriticalEnergy() * pow(fabs(m_electronEnergy), 3) / m_bendingRadius;

    m_totalPower = get_factorTotalPowerDipol() * 0.1 * pow(fabs(m_electronEnergy), 3) * m_magneticFieldStrength * fabs(m_horDivergence) / 1000.0;
    m_gamma = m_electronEnergy / (c_electronMass * pow(c_speedOfLight, 2) / (c_electronVolt)*1.e-9);

    if (m_gamma >= 1) {
        m_beta = sqrt(pow(m_gamma, 2) - 1) / m_gamma;
    } else {
        m_beta = 1;
    }
}

void DipoleSource::calcPhotonWavelength() {
    // Energy Distribution Type : Values only
    m_photonWaveLength = m_photonEnergy == 0.0 ? 0 : inm2eV / m_photonEnergy;
}

void DipoleSource::calcSourcePath() {
    m_sourcePathLength = fabs(m_sourcePulseLength) * 1000 * 0.3;
    m_phaseJitter = m_photonEnergy == 0 ? 0 : fabs(m_sourcePulseLength * 0.3) / m_photonWaveLength * 2000000 * PI;
}

void DipoleSource::calcHorDivDegSec() {
    m_horDivDegrees = m_horDivergence * 0.180 / PI;
    m_horDivSeconds = m_horDivergence * 0.180 / PI * 3600;
}

void DipoleSource::calcFluxOrg() {
    //double phi0 = m_horDivergence * 1.e-3;
    //double m_magneticField = m_magneticFieldStrength / 1000;
    double bandwidth = 0.001;

    if (m_energySpread != 0.0) {
        bandwidth = m_energySpread / 100;
    }

    m_flux = (m_stokes[2] + m_stokes[3]) * (m_horDivergence * 1e-3) * bandwidth * 100.0;
}

void DipoleSource::setMaxEnergy() {
    double EMAXS = 285.81224786 * m_criticalEnergy;
    double Emax = m_electronEnergy + m_energySpread / 2;
    double Emin = m_electronEnergy - m_energySpread / 2;

    if (Emax < EMAXS) {
        m_maxFlux = Emax;
    } else if (Emin > EMAXS) {
        m_maxFlux = Emin;
    } else {
        m_maxFlux = EMAXS;
    }
}

double DipoleSource::getInterpolation(double energy) const {
    //TODO: Interpolation benchmarken 
    double functionOne = 0.0;
    double functionTwo = 0.0;
    double result = 0.0;
    int x0Position = 0;

    for (int i = 0; i < int(m_schwingerX.size()); i++) {
        if (energy < m_schwingerX[i]) {
            break;
        }
        x0Position++;  // TODO out of bounds checken
    }

    double dx0 = energy - m_schwingerX[x0Position - 1];
    double dx1 = energy - m_schwingerX[x0Position];
    double dx2 = energy - m_schwingerX[x0Position + 1];

    functionOne = (dx0 * m_schwingerY[x0Position] - dx1 * m_schwingerY[x0Position - 1]) / (dx0 - dx1);
    functionTwo = (dx0 * m_schwingerY[x0Position + 1] - dx2 * m_schwingerY[x0Position - 1]) / (dx0 - dx2);

    result = (dx1 * functionTwo - dx2 * functionOne) / (dx1 - dx2);

    return result;
}

}  // namespace RAYX
