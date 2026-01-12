#ifndef DESCRIPTOR_POOL_H
#define DESCRIPTOR_POOL_H

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "vulkan_api/utils/defines.h"


typedef struct
{
    VkDescriptorPool handle;

    struct
    {
        VkDescriptorType* data;
        uint32_t size;
    } types;

} DescriptorPool;


bool DescriptorPool_create(DescriptorPool* pool, const VkDescriptorPoolSize* poolSizes, uint32_t count, VkDevice device);
bool DescriptorPool_allocateDescriptorSets(DescriptorPool* pool, VkDescriptorSet* descriptorSets, const VkDescriptorSetLayout* layouts, VkDevice device);
void DescriptorPool_writeCombinedImageSampler(DescriptorPool* pool, const VkDescriptorImageInfo* imageInfo, VkDescriptorSet descriptorSet, uint32_t dstBinding, VkDevice device);
void DescriptorPool_destroy(DescriptorPool* pool, VkDevice device);


#endif // !DESCRIPTOR_POOL_H