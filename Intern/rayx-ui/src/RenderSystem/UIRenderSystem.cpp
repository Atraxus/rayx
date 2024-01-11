#include "UIRenderSystem.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <nfd.h>

#include <fstream>
#include <rapidxml.hpp>

#include "CanonicalizePath.h"
#include "RayProcessing.h"

void checkVkResult(VkResult result, const char* message) {
    if (result != VK_SUCCESS) {
        printf("%s\n", message);
        exit(1);
    }
}

// ---- UIRenderSystem ----
UIRenderSystem::UIRenderSystem(const Window& window, const Device& device, VkFormat imageFormat, VkFormat depthFormat, uint32_t imageCount)
    : m_Window(window), m_Device(device) {
    // Create descriptor pool for IMGUI
    VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    if (vkCreateDescriptorPool(m_Device.device(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create imgui descriptor pool");
    }

    // Create render pass for IMGUI
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = imageFormat;  // same as in main render pass
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // same as in main render pass
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;    // same as in main render pass

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = depthFormat;  // same as in main render pass
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                       // same as in main render pass
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;  // same as in main render pass

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments = attachments.data();
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    if (vkCreateRenderPass(m_Device.device(), &info, nullptr, &m_RenderPass) != VK_SUCCESS) {
        throw std::runtime_error("Could not create Dear ImGui's render pass");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_IO = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = m_Device.instance();
    initInfo.PhysicalDevice = m_Device.physicalDevice();
    initInfo.Device = m_Device.device();
    initInfo.QueueFamily = m_Device.findPhysicalQueueFamilies().graphicsFamily;
    initInfo.Queue = m_Device.graphicsQueue();
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = m_DescriptorPool;
    initInfo.Allocator = nullptr;
    initInfo.MinImageCount = imageCount;
    initInfo.ImageCount = imageCount;
    initInfo.CheckVkResultFn = nullptr;

    ImGui_ImplGlfw_InitForVulkan(m_Window.window(), true);
    ImGui_ImplVulkan_Init(&initInfo, m_RenderPass);

    // Upload fonts
    {
        // Setup style
        m_smallFont =
            m_IO.Fonts->AddFontFromFileTTF(RAYX::canonicalizeRepositoryPath("./Intern/rayx-ui/res/fonts/Roboto-Regular.ttf").string().c_str(), 16.0f);
        m_largeFont =
            m_IO.Fonts->AddFontFromFileTTF(RAYX::canonicalizeRepositoryPath("./Intern/rayx-ui/res/fonts/Roboto-Regular.ttf").string().c_str(), 24.0f);

        auto tmpCommandBuffer = m_Device.beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture(tmpCommandBuffer);
        m_Device.endSingleTimeCommands(tmpCommandBuffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

UIRenderSystem::~UIRenderSystem() {
    vkDestroyRenderPass(m_Device.device(), m_RenderPass, nullptr);
    vkDestroyDescriptorPool(m_Device.device(), m_DescriptorPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
/**
 * @brief
 *
 * @param uiParams
 * @param rObjects
 */
void UIRenderSystem::setupUI(UIParameters& uiParams, std::vector<RAYX::OpticalElement>& elemets, std::vector<glm::dvec3>& rSourcePositions) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (m_useLargeFont) {
        ImGui::PushFont(m_largeFont);
    } else {
        ImGui::PushFont(m_smallFont);
    }

    showSceneEditorWindow(uiParams);
    showMissingFilePopupWindow(uiParams);
    showSettingsWindow();
    showHotkeysWindow();
    m_BeamlineOutliner.showBeamlineOutlineWindow(uiParams, elemets, rSourcePositions);

    ImGui::PopFont();
}

void UIRenderSystem::render(VkCommandBuffer commandBuffer) {
    ImGui::Render();

    ImDrawData* drawData = ImGui::GetDrawData();

    // Avoid rendering when minimized
    if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f) {
        return;
    }

    // Create and submit command buffers
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}

void UIRenderSystem::showSceneEditorWindow(UIParameters& uiParams) {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(450, 450), ImGuiCond_Once);

    ImGui::Begin("Properties Manager");

    if (ImGui::Button("Open File Dialog")) {
        nfdchar_t* outPath;
        constexpr uint32_t filterCount = 1;
        nfdfilteritem_t filterItem[filterCount] = {{"RML Files", "rml, xml"}};
        nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, filterCount, NULL);
        if (result == NFD_OKAY) {
            std::string rmlPath = outPath;
            std::string rayFilePathH5 = rmlPath.substr(0, rmlPath.size() - 4) + ".h5";
            std::string rayFilePathCSV = rmlPath.substr(0, rmlPath.size() - 4) + ".csv";

#ifndef NO_H5
            uiParams.showH5NotExistPopup = !std::filesystem::exists(rayFilePathH5);
#else
            uiParams.showH5NotExistPopup = !std::filesystem::exists(rayFilePathCSV);
#endif
            uiParams.showRMLNotExistPopup = !std::filesystem::exists(rmlPath);

            if (uiParams.showH5NotExistPopup || uiParams.showRMLNotExistPopup) {
                uiParams.pathChanged = false;
            } else {
                uiParams.pathChanged = true;
                uiParams.pathValidState = true;
                uiParams.rmlPath = outPath;
            }
        } else if (result == NFD_CANCEL) {
            puts("User pressed cancel.");
        } else {
            printf("Error: %s\n", NFD_GetError());
        }
    }

    ImGui::Text("Background");
    ImGui::ColorEdit3("Color", (float*)&m_ClearColor);

    ImGui::Separator();
    uiParams.camController.displaySettings();
    ImGui::Separator();
    if (!uiParams.rmlPath.empty() && uiParams.pathValidState) {
        size_t tempAmountOfRays = uiParams.rayInfo.amountOfRays;
        bool tempDisplayRays = uiParams.rayInfo.displayRays;
        bool tempRenderAllRays = uiParams.rayInfo.renderAllRays;
        displayFilterSlider(uiParams.rayInfo.amountOfRays, uiParams.rayInfo.maxAmountOfRays, uiParams.rayInfo.displayRays,
                            uiParams.rayInfo.renderAllRays);
        if (tempAmountOfRays != uiParams.rayInfo.amountOfRays || tempDisplayRays != uiParams.rayInfo.displayRays ||
            tempRenderAllRays != uiParams.rayInfo.renderAllRays) {
            uiParams.rayInfo.raysChanged = true;
        }
        if (tempRenderAllRays != uiParams.rayInfo.renderAllRays) {
            uiParams.rayInfo.cacheChanged = true;
        }
    }
    ImGui::Text("Application average %.6f ms/frame", uiParams.frameTime * 1000.0f);

    ImGui::End();
}

void UIRenderSystem::showSettingsWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 450), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(450, 100), ImGuiCond_Once);

    ImGui::Begin("Settings");

    ImGui::Checkbox("Large Font", &m_useLargeFont);

    ImGui::End();
}

void UIRenderSystem::showHotkeysWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 550), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(450, 210), ImGuiCond_Once);

    ImGui::Begin("Hotkeys");

    ImGui::Text("Keyboard Hotkeys:");
    ImGui::Separator();
    ImGui::Text("ESC: Close the window");
    ImGui::Text("F11: Fullscreen mode");
    ImGui::Text("F10: Windowed mode");
    ImGui::Separator();
    ImGui::Text("W, A, S, D: Camera movement (along plane)");
    ImGui::Text("Q: Move camera downward");
    ImGui::Text("E: Move camera upward");
    ImGui::Separator();
    ImGui::Text("Hold SHIFT for faster movement");

    ImGui::End();
}

void UIRenderSystem::showMissingFilePopupWindow(UIParameters& uiParams) {
    if (uiParams.showH5NotExistPopup || uiParams.showRMLNotExistPopup) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always,
                                ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);  // Set size

        ImGui::OpenPopup("File Not Found");
        if (ImGui::BeginPopupModal("File Not Found", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            // Scale up font size
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

            if (uiParams.showH5NotExistPopup && uiParams.showRMLNotExistPopup) {
                ImGui::Text("Both RML and H5 files do not exist.");
            } else if (uiParams.showH5NotExistPopup) {
                ImGui::Text("The H5 file does not exist.");
            } else {
                ImGui::Text("The RML file does not exist.");
            }
            ImGui::Spacing();
            if (ImGui::Button("OK")) {
                uiParams.showH5NotExistPopup = false;
                uiParams.showRMLNotExistPopup = false;
            }

            // Revert to original font size
            ImGui::PopFont();

            ImGui::EndPopup();
        }
    }
}
