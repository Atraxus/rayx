#ifndef BEHAVIOUR_H
#define BEHAVIOUR_H

// A behaviour decides what happens whenever a ray hits the surface of this element.
// Each behaviour type has its own `trace` function.
const int BTYPE_MIRROR = 0;
const int BTYPE_GRATING = 1;
const int BTYPE_SLIT = 2;
const int BTYPE_RZP = 3;
const int BTYPE_IMAGE_PLANE = 4;

struct Behaviour {
    // the type of this behaviour, see the BTYPE constants.
    // the type describes how the m_params need to be interpreted.
    double m_type;
    double m_params[16];
};

#endif
