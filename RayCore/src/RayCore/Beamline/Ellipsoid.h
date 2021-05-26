#pragma once
#include "Quadric.h"

namespace RAY
{

    class RAY_API Ellipsoid : public Quadric {

    public:
        
        Ellipsoid(const char* name, double width, double height, double grazingIncidence, double azimuthal, double distanceToPreceedingElement, 
            double entranceArmLength, double exitArmLength, int coordSys, int figRot, double a11, std::vector<double> misalignmentParams);
        Ellipsoid();
        ~Ellipsoid();

        void calcRadius();
        void calcHalfAxes();
        void calcAlphaBeta(double grazinIncidence);
        double getWidth();
        double getHeight();
        double getRadius();
        double getExitArmLength();
        double getEntranceArmLength();
        double getMy0(); // center of ellipsoid
        double getMz0(); // -"-
        // grazing incidence, in rad
        double getAlpha();
        double getBeta();
        double getChi();
        double getDistanceToPreceedingElement();
        double getShortHalfAxisB(); // b
        double getLongHalfAxisA(); // a
        double getOffsetY0(); // always = 0?
        double getAlpha1(); // from tangent angle and grazing incidence???
        // derived params
        double getTangentAngle();
        double getA34();
        double getA33();
        double getA44();
        double getHalfAxisC();
        
    private:
        double m_totalWidth;
        double m_totalHeight;
        double m_radius;
        double m_exitArmLength;
        double m_entranceArmLength;
        double m_y0; // center of ellipsoid
        double m_z0; // -"-
        // grazing incidence, in rad
        double m_alpha;
        double m_beta;
        double m_chi;
        double m_distanceToPreceedingElement;
        double m_shortHalfAxisB; // b
        double m_longHalfAxisA; // a
        double m_offsetY0; // always = 0?
        double m_alpha1; // from tangent angle and grazing incidence???
        // derived params
        double d_tangentAngle;
        double d_a34;
        double d_a33;
        double d_a44;
        
        enum FIGURE_ROTATION  { FR_YES, FR_PLANE, FR_A11};
        FIGURE_ROTATION m_figureRotation;
        enum COORDINATESYSTEM_TYPE {CS_CURVATURE, CS_MIRROR};
        COORDINATESYSTEM_TYPE m_misalignmentCoordSys;
        double m_a11; // param for quadric funciton, given by user
        
        double d_halfAxisC; // derived from figure_rotation, a_11 and half axes a, b
        //double m_Depth;
        //double m_verDivergence;
        //double m_horDivergence;
        
    };

} // namespace RAY