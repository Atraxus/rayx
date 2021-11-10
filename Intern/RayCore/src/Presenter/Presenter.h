#pragma once

#include <memory>

#include "Core.h"
#include "Model/Beamline/Beamline.h"
#include "Model/Beamline/LightSource.h"
#include "Tracer/TracerInterface.h"

namespace RAYX {
class RAYX_API Presenter {
  public:
    Presenter();
    Presenter(std::shared_ptr<Beamline> beamline);
    ~Presenter();

    bool run();

    void addLightSource(std::shared_ptr<LightSource> newSource);

  private:
    std::shared_ptr<Beamline> m_Beamline;
    TracerInterface m_TracerInterface;
};

}  // namespace RAYX
