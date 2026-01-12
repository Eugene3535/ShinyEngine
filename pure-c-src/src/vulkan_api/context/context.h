#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include <stdbool.h>

#include <vulkan/vulkan.h>


typedef struct
{
    VkInstance       instance;
    VkPhysicalDevice GPU;
    VkDevice         device;
    VkQueue          queue;
    uint32_t         mainQueueFamilyIndex;
} VulkanContext;


bool VulkanContext_create(VulkanContext* context);
void VulkanContext_destroy(VulkanContext* context);


#endif // !VULKAN_CONTEXT_H