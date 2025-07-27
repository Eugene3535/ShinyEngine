#include <vulkan/vulkan.h>

#include "vulkan_api/wrappers/logical_device/LogicalDevice.hpp"
#include "vulkan_api/wrappers/command_pool/CommandBufferPool.hpp"


CommandBufferPool::CommandBufferPool() noexcept:
    handle(nullptr)
{

}


CommandBufferPool::~CommandBufferPool() = default;


bool CommandBufferPool::create(const LogicalDevice& logicalDevice) noexcept
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = logicalDevice.mainQueueFamilyIndex;

    if (vkCreateCommandPool(logicalDevice.handle, &poolInfo, nullptr, &handle) != VK_SUCCESS)
        return false;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = handle;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    return (vkAllocateCommandBuffers(logicalDevice.handle, &allocInfo, commandBuffers.data()) == VK_SUCCESS);
}


void CommandBufferPool::destroy(VkDevice logicalDevice) noexcept
{
    vkDestroyCommandPool(logicalDevice, handle, nullptr);
}