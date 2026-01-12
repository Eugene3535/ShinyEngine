#include <vulkan/vulkan.h>

#include "vulkan_api/command_pool/command_buffer_pool.h"


bool CommandBufferPool_create(CommandBufferPool* pool, VkDevice device, uint32_t queueFamilyIndex)
{
    const VkCommandPoolCreateInfo poolInfo = 
    {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = VK_NULL_HANDLE,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndex
    };

    if (vkCreateCommandPool(device, &poolInfo, VK_NULL_HANDLE, &pool->handle) != VK_SUCCESS)
        return false;

    const VkCommandBufferAllocateInfo allocInfo = 
    {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = VK_NULL_HANDLE,
        .commandPool        = pool->handle,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = sizeof(pool->commandBuffers) / sizeof(VkCommandBuffer)
    };

    return (vkAllocateCommandBuffers(device, &allocInfo, pool->commandBuffers) == VK_SUCCESS);
}


void CommandBufferPool_destroy(CommandBufferPool* cbp, VkDevice device)
{
    vkDestroyCommandPool(device, cbp->handle, VK_NULL_HANDLE);
}