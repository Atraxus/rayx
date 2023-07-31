#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <glm/gtx/transform.hpp>

#include "Device.h"
#include "Swapchain.h"

class ImGuiLayer {
  public:
    ImGuiLayer(const Window& window, const Device& device, const SwapChain& swapchain);
    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;
    ~ImGuiLayer();

    void updateImGui();
    VkCommandBuffer recordImGuiCommands(uint32_t currentImage, const VkFramebuffer framebuffer, const VkExtent2D& extent);

    VkCommandBuffer getCommandBuffer(uint32_t index) const { return m_CommandBuffers[index]; }
    VkClearValue getClearValue() const { return {m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]}; }

  private:
    const Window& m_Window;
    const Device& m_Device;

    bool m_LayerEnabled = true;
    bool m_ShowDemoWindow = false;
    float m_ClearColor[4] = {0.01f, 0.01f, 0.01f, 1.00f};

    VkRenderPass m_RenderPass;
    VkDescriptorPool m_DescriptorPool;
    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    ImGuiIO m_IO;

    void createCommandPool();
    void createCommandBuffers(uint32_t cmdBufferCount);
};