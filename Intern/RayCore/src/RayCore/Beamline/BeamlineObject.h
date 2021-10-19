#pragma once
#include "Core.h"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <memory>
#include "rapidxml.hpp"


namespace RAYX
{
    /*
    * Brief: Abstract parent class for all beamline objects used in Ray-X.
    *
    */
    class RAYX_API BeamlineObject
    {
    public:
        virtual ~BeamlineObject(); // TODO(rudi) remove virtual, this is a temporary hotfix to allow dynamic casting

        const char* getName() const;

        const int m_ID;

    protected:
        BeamlineObject(const char* name);
        BeamlineObject();

    private:
        const char* m_name;
        //m_geometry;
        //m_surface; //(for lightsource??)


    };


} // namespace RAYX