#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <unordered_set>
#include <optional>
#include <set>

#include "vulkan_api/utils/Defines.hpp"
#include "vulkan_api/utils/Constants.hpp"
#include "vulkan_api/wrappers/instance/VulkanInstance.hpp"
#include "vulkan_api/wrappers/physical_device/PhysicalDevice.hpp"
#include "vulkan_api/wrappers/logical_device/LogicalDevice.hpp"
#include "vulkan_api/wrappers/surface/Surface.hpp"
#include "vulkan_api/wrappers/swapchain/Swapchain.hpp"
#include "vulkan_api/wrappers/graphics_pipeline/GraphicsPipeline.hpp"
#include "vulkan_api/wrappers/command_pool/CommandBufferPool.hpp"
#include "vulkan_api/wrappers/sync/SyncManager.hpp"
#include "vulkan_api/wrappers/texture/Texture2D.hpp"
#include "vulkan_api/wrappers/mesh/Mesh.hpp"
#include "vulkan_api/wrappers/uniform/UniformBufferHolder.hpp"


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;



class Application
{
public:
    void run() noexcept;

private:
    void initWindow() noexcept;
    bool initVulkan() noexcept;
    void mainLoop() noexcept;
    void cleanup() noexcept;
    void recreateSwapChain() noexcept;

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) noexcept;
    void updateUniformBuffer(uint32_t currentImage) noexcept;
    void drawFrame() noexcept;

    GLFWwindow* window;

    VulkanInstance    m_instance;
    PhysicalDevice    m_physicalDevice;
    LogicalDevice     m_logicalDevice;
    Surface           m_surface;
    Swapchain         m_swapchain;
    GraphicsPipeline  m_pipeline;
    CommandBufferPool m_commandPool;
    SyncManager       m_sync;

    Texture2D m_texture;
    Mesh      m_mesh;

    UniformBufferHolder m_uniformBuffers;

    bool framebufferResized = false;
};

#endif // !APPLICATION_HPP