#ifndef COMMAND_BUFFER_POOL_H
#define COMMAND_BUFFER_POOL_H

#include <stdbool.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include "vulkan_api/utils/defines.h"


typedef struct
{
    VkCommandPool handle;
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
} CommandBufferPool;


bool CommandBufferPool_create(CommandBufferPool* pool, VkDevice device, uint32_t queueFamilyIndex);
void CommandBufferPool_destroy(CommandBufferPool* pool, VkDevice device);

#endif // !COMMAND_BUFFER_POOL_H