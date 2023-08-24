#include "LineRenderSystem.h"

LineRenderSystem::LineRenderSystem(Device& device, Scene& scene, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    : m_Device(device), m_Scene(scene) {
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

LineRenderSystem::~LineRenderSystem() { vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr); }

void LineRenderSystem::render(FrameInfo& frameInfo) {
    m_Pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &frameInfo.descriptorSet, 0, nullptr);

    m_Scene.bind(frameInfo.commandBuffer, Scene::Topography::LINE_TOPOGRAPHY);
    m_Scene.draw(frameInfo.commandBuffer, Scene::Topography::LINE_TOPOGRAPHY);
}

void LineRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void LineRenderSystem::createPipeline(VkRenderPass renderPass) {
    assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    GraphicsPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
    // pipelineConfig.rasterizationInfo.lineWidth = 2.0f;
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_PipelineLayout;
    m_Pipeline = std::make_unique<GraphicsPipeline>(m_Device, "../../../Intern/rayx-ui/src/Shaders/vert.spv",
                                                    "../../../Intern/rayx-ui/src/Shaders/frag.spv", pipelineConfig);
}