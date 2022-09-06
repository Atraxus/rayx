#include "VulkanTracer.h"

#include <Material/Material.h>

#include <chrono>
#include <cmath>

#include "Debug.h"
#include "Debug/Instrumentor.h"
#include "PathResolver.h"

#ifdef RAYX_PLATFORM_MSVC
#ifdef USE_NSIGHT_AFTERMATH
#include "GFSDK_Aftermath.h"
#endif
#endif

namespace RAYX {
VulkanTracer::VulkanTracer() {
    // Set buffer settings (DEBUG OR RELEASE)
    RAYX_LOG << "Initializing Vulkan Tracer..";
    setSettings();

    dict<BufferSpec> bs = {
        {"ray-buffer", {.binding = 0, .in = true, .out = false}},
        {"output-buffer", {.binding = 1, .in = false, .out = true}},
        {"quadric-buffer", {.binding = 2, .in = true, .out = false}},
        {"xyznull-buffer",
         {.binding = 3,
          .in = true,
          .out = false}},  // TODO what is this buffer?
        {"material-index-table", {.binding = 4, .in = true, .out = false}},
        {"material-table", {.binding = 5, .in = true, .out = false}},
#ifdef RAYX_DEBUG_MODE
        {"debug-buffer", {.binding = 6, .in = false, .out = true}},
#endif
    };
    m_engine.init({.shaderfile = "build/bin/comp.spv", .bufferSpecs = bs});
}

VulkanTracer::~VulkanTracer() { cleanup(); }

RayList VulkanTracer::trace(const Beamline& beamline) {
    m_RayList = beamline.getInputRays();

    setBeamlineParameters(1, beamline.m_OpticalElements.size(),
                          m_RayList.rayAmount());

    for (const auto& e : beamline.m_OpticalElements) {
        addArrays(e->getSurfaceParams(), glmToArray16(e->getInMatrix()),
                  glmToArray16(e->getOutMatrix()), e->getObjectParameters(),
                  e->getElementParameters());
    }

    m_MaterialTables = beamline.calcMinimalMaterialTables();

    std::vector<Ray> rays;
    for (auto r : m_RayList) {
        rays.push_back(r);
    }
    dict<GpuData> buffers = {
        {"ray-buffer", encode(rays)},
        {"quadric-buffer", encode(m_beamlineData)},
        {"material-index-table", encode(m_MaterialTables.indexTable)},
        {"material-table", encode(m_MaterialTables.materialTable)},
    };

    RunSpec r = {.numberOfInvocations = m_numberOfRays,
                 .computeBuffersCount = m_settings.m_computeBuffersCount,
                 .buffers = buffers};

    auto outputbuffers = m_engine.run(r);
    std::vector<Ray> outrays = decode<Ray>(outputbuffers["output-buffer"]);
    RayList outraylist;
    for (auto r : outrays) {
        outraylist.push(r);
    }

    m_debugBufList = decode<_debugBuf_t>(outputbuffers["debug-buffer"]);

    cleanTracer();

    return outraylist;
}

//	This function destroys the debug messenger
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
    RAYX_PROFILE_FUNCTION();
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

/** Cleans and deletes the whole tracer instance. Do this only if you do not
 * want to reuse the instance anymore
 * CALL CLEANTRACER FIRST BEFORE CALLING THIS ONE
 */
void VulkanTracer::cleanup() {
    RAYX_PROFILE_FUNCTION();
    vkDestroyDescriptorSetLayout(m_engine.m_Device,
                                 m_engine.m_DescriptorSetLayout, nullptr);
    vkDestroyCommandPool(m_engine.m_Device, m_engine.m_CommandPool, nullptr);
    {
        RAYX_PROFILE_SCOPE("vkDestroyDevice");
        vkDestroyDevice(m_engine.m_Device, nullptr);
    }
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_engine.m_Instance,
                                      m_engine.m_DebugMessenger, nullptr);
    }
    {
        RAYX_PROFILE_SCOPE("vkDestroyInstance");
        vkDestroyInstance(m_engine.m_Instance, nullptr);
    }
}

/**
 * Use this function if you want to reuse the tracer instance with a new
 * beamline and new rays etc but do not want to initialize everything again
 */
void VulkanTracer::cleanTracer() {
    m_RayList.clean();
    m_beamlineData.clear();
    vkFreeCommandBuffers(m_engine.m_Device, m_engine.m_CommandPool, 1,
                         &m_engine.m_CommandBuffer);
    vkDestroyPipeline(m_engine.m_Device, m_engine.m_Pipeline, nullptr);
    vkDestroyPipelineLayout(m_engine.m_Device, m_engine.m_PipelineLayout,
                            nullptr);
    vkDestroyDescriptorPool(m_engine.m_Device, m_engine.m_DescriptorPool,
                            nullptr);

    for (uint32_t i = 0; i < m_engine.m_compute.m_Buffers.size(); i++) {
        vkDestroyBuffer(m_engine.m_Device, m_engine.m_compute.m_Buffers[i],
                        nullptr);
        vkFreeMemory(m_engine.m_Device, m_engine.m_compute.m_BufferMemories[i],
                     nullptr);
    }
    for (uint32_t i = 0; i < m_engine.m_staging.m_Buffers.size(); i++) {
        vkDestroyBuffer(m_engine.m_Device, m_engine.m_staging.m_Buffers[i],
                        nullptr);
        vkFreeMemory(m_engine.m_Device, m_engine.m_staging.m_BufferMemories[i],
                     nullptr);
    }
    vkDestroyShaderModule(m_engine.m_Device, m_engine.m_ComputeShaderModule,
                          nullptr);
}

void VulkanTracer::setBeamlineParameters(uint32_t inNumberOfBeamlines,
                                         uint32_t inNumberOfQuadricsPerBeamline,
                                         uint32_t inNumberOfRays) {
    RAYX_PROFILE_FUNCTION();
    RAYX_LOG << "Setting Beamline Parameters:";
    RAYX_LOG << "\tNumber of beamlines: " << inNumberOfBeamlines;
    RAYX_LOG << "\tNumber of Quadrics/Beamline: "
             << inNumberOfQuadricsPerBeamline;
    RAYX_LOG << "\tNumber of Rays: " << inNumberOfRays;
    m_numberOfBeamlines = inNumberOfBeamlines;
    m_numberOfQuadricsPerBeamline = inNumberOfQuadricsPerBeamline;
    m_numberOfRays = inNumberOfRays * inNumberOfBeamlines;
    m_numberOfRaysPerBeamline = inNumberOfRays;
    if (m_beamlineData.size() < 4) {
        m_beamlineData.resize(4);
    }
    m_beamlineData[0] = m_numberOfBeamlines;
    m_beamlineData[1] = m_numberOfQuadricsPerBeamline;
    m_beamlineData[2] = m_numberOfRays;
    m_beamlineData[3] = m_numberOfRaysPerBeamline;
}

void VulkanTracer::addRayVector(std::vector<Ray>&& inRayVector) {
    RAYX_PROFILE_FUNCTION();

    RAYX_LOG << "Inserting into rayList. rayList.rayAmount() before: "
             << m_RayList.rayAmount();
    RAYX_LOG << "Sent size: " << inRayVector.size();
    m_RayList.insertVector(inRayVector);
}

// adds quad to beamline
void VulkanTracer::addArrays(
    const std::array<double, 4 * 4>& surfaceParams,
    const std::array<double, 4 * 4>& inputInMatrix,
    const std::array<double, 4 * 4>& inputOutMatrix,
    const std::array<double, 4 * 4>& objectParameters,
    const std::array<double, 4 * 4>& elementParameters) {
    RAYX_PROFILE_FUNCTION();
    // beamline.resize(beamline.size()+1);

    m_beamlineData.insert(m_beamlineData.end(), surfaceParams.begin(),
                          surfaceParams.end());
    m_beamlineData.insert(m_beamlineData.end(), inputInMatrix.begin(),
                          inputInMatrix.end());
    m_beamlineData.insert(m_beamlineData.end(), inputOutMatrix.begin(),
                          inputOutMatrix.end());
    m_beamlineData.insert(m_beamlineData.end(), objectParameters.begin(),
                          objectParameters.end());
    m_beamlineData.insert(m_beamlineData.end(), elementParameters.begin(),
                          elementParameters.end());
}

uint32_t VulkanTracer::getNumberOfBuffers() const {
    return m_settings.m_buffersCount;
}
bool VulkanTracer::isDebug() const { return m_settings.m_isDebug; }

// Set Vulkan Tracer m_settings according to Release or Debug Mode
void VulkanTracer::setSettings() {
#ifdef RAYX_DEBUG_MODE
    RAYX_D_LOG << "VulkanTracer Debug: ON";
    m_settings.m_isDebug = true;
    m_settings.m_computeBuffersCount = 7;
    m_settings.m_stagingBuffersCount = 2;
#else
    m_settings.m_isDebug = false;
    m_settings.m_computeBuffersCount = 6;
    m_settings.m_stagingBuffersCount = 1;
#endif
    m_settings.m_buffersCount =
        m_settings.m_computeBuffersCount + m_settings.m_stagingBuffersCount;
}
}  // namespace RAYX
