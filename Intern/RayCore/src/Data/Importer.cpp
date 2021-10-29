#include <fstream>
#include <sstream>

#include "rapidxml.hpp"

#include "Model/Beamline/Objects/PointSource.h"
#include "Model/Beamline/Objects/MatrixSource.h"
#include "Model/Beamline/Objects/ImagePlane.h"
#include "Model/Beamline/Objects/PlaneMirror.h"
#include "Model/Beamline/Objects/ToroidMirror.h"
#include "Model/Beamline/Objects/Slit.h"
#include "Model/Beamline/Objects/SphereGrating.h"
#include "Model/Beamline/Objects/PlaneGrating.h"
#include "Model/Beamline/Objects/SphereMirror.h"
#include "Model/Beamline/Objects/ReflectionZonePlate.h"

#include "Importer.h"
#include <string.h>

namespace RAYX
{
    Importer::Importer()
    {

    }

    Importer::~Importer()
    {

    }

    void addBeamlineObjectFromXML(rapidxml::xml_node<>* node, Beamline* beamline) {
        const char* type = node->first_attribute("type")->value();

        // the following three blocks of code are lambda expressions (see https://en.cppreference.com/w/cpp/language/lambda)
        // They define functions to be used in the if-else-chain below to keep it structured and readable.

        // addLightSource(s, node) is a function adding a light source to the beamline (if it's not nullptr)
        const auto addLightSource = [&](std::shared_ptr<LightSource> s, rapidxml::xml_node<>* node) {
            if (s) {
                beamline->m_LightSources.push_back(s);
            } else {
                std::cerr << "could not construct LightSource with Name: " << node->first_attribute("name")->value() << "; Type: " << node->first_attribute("type")->value() << '\n';
            }
        };

        // addOpticalElement(e, node) is a function adding an optical element to the beamline (if it's not nullptr)
        const auto addOpticalElement = [&](std::shared_ptr<OpticalElement> e, rapidxml::xml_node<>* node) {
            if (e) {
                beamline->m_OpticalElements.push_back(e);
            } else {
                std::cerr << "could not construct OpticalElement with Name: " << node->first_attribute("name")->value() << "; Type: " << node->first_attribute("type")->value() << '\n';
            }
        };

        // every beamline object has a function createFromXML which constructs the object from a given xml-node if possible (otherwise it will return a nullptr)
        // The createFromXML functions use the param* functions declared in <Data/xml.h>
        if (strcmp(type, "Point Source") == 0) {
            addLightSource(PointSource::createFromXML(node), node);
        } else if (strcmp(type, "Matrix Source") == 0) {
            addLightSource(MatrixSource::createFromXML(node), node);
        } else if (strcmp(type, "ImagePlane") == 0) {
            addOpticalElement(ImagePlane::createFromXML(node), node);
        } else if (strcmp(type, "Plane Mirror") == 0) {
            addOpticalElement(PlaneMirror::createFromXML(node), node);
        } else if (strcmp(type, "Toroid") == 0) {
            addOpticalElement(ToroidMirror::createFromXML(node), node);
        } else if (strcmp(type, "Slit") == 0) {
            addOpticalElement(Slit::createFromXML(node, 0.0), node); // TODO(rudi): sourceEnergy for slit!
        } else if (strcmp(type, "Spherical Grating") == 0) {
            addOpticalElement(SphereGrating::createFromXML(node), node);
        } else if (strcmp(type, "Plane Grating") == 0) {
            addOpticalElement(PlaneGrating::createFromXML(node), node);
        } else if (strcmp(type, "Sphere") == 0) {
            addOpticalElement(SphereMirror::createFromXML(node), node);
        } else if (strcmp(type, "Reflection Zoneplate") == 0) {
            addOpticalElement(ReflectionZonePlate::createFromXML(node), node);
        } else {
            std::cerr << "could not classify beamline object with Name: " << node->first_attribute("name")->value() << "; Type: " << node->first_attribute("type")->value() << '\n';
        }
    }

    Beamline Importer::importBeamline(const char* filename) {
        // first implementation: stringstreams are slow; this might need optimization
        std::cout << "importBeamline is called with file \"" << filename << "\"\n";

        std::ifstream t(filename);
        std::stringstream buffer;
        buffer << t.rdbuf();
        std::string test = buffer.str();
        std::vector<char> cstr(test.c_str(), test.c_str() + test.size() + 1);
        rapidxml::xml_document<> doc;
        doc.parse<0>(cstr.data());

        std::cout << "Version: " << doc.first_node("lab")->first_node("version")->value() << std::endl;
        rapidxml::xml_node<>* xml_beamline = doc.first_node("lab")->first_node("beamline");

        Beamline beamline;

        for (rapidxml::xml_node<>* object = xml_beamline->first_node(); object; object = object->next_sibling()) { // Iterating through objects
            addBeamlineObjectFromXML(object, &beamline);
        }
        return beamline;
    }

} // namespace RAYX