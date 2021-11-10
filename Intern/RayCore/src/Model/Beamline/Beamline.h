#pragma once

#include <memory>
#include <vector>

#include "Core.h"
#include "glm.hpp"

namespace RAYX {
class OpticalElement;
class LightSource;

class RAYX_API Beamline {
  public:
    Beamline();
    ~Beamline();

    void addOpticalElement(const std::shared_ptr<OpticalElement> q);
    void addOpticalElement(const char* name,
                           const std::vector<double>& inputPoints,
                           std::vector<double> inputInMatrix,
                           std::vector<double> inputOutMatrix,
                           std::vector<double> OParameters,
                           std::vector<double> EParameters);
    void addOpticalElement(const char* name, std::vector<double>&& inputPoints,
                           std::vector<double>&& inputInMatrix,
                           std::vector<double>&& inputOutMatrix,
                           std::vector<double>&& OParameters,
                           std::vector<double>&& EParameters);

    std::vector<std::shared_ptr<OpticalElement>> m_OpticalElements;
    std::vector<std::shared_ptr<LightSource>> m_LightSources;
};

}  // namespace RAYX