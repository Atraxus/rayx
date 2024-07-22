#ifndef REFRAC_H
#define REFRAC_H

#include "Adapt.h"

/**
calculates refracted ray
@params: 	r: ray
            normal: normal at intersection point of ray and element -> for planes normal is always the same (0,1,0) -> no rotation and thus no trigonometric
            az: line spacing in z direction
            ax: line spacing in x direction
@returns: refracted ray (position unchanged, direction changed), weight = ETYPE_BEYOND_HORIZON if
"ray beyond horizon"
*/
Ray refrac2D(Ray r, dvec3 normal, double az, double ax);


#endif
