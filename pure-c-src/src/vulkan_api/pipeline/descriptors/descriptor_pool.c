#include "vulkan_api/pipeline/descriptors/descriptor_pool.h"


bool DescriptorPool_create(DescriptorPool* pool, const VkDescriptorPoolSize* poolSizes, uint32_t count, VkDevice device)
{
    const VkDescriptorPoolCreateInfo poolInfo = 
    {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = VK_NULL_HANDLE,
        .flags         = 0,
        .maxSets       = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = count,
        .pPoolSizes    = poolSizes
    };

    bool result = (vkCreateDescriptorPool(device, &poolInfo, VK_NULL_HANDLE, &pool->handle) == VK_SUCCESS);

    if(result)
    {
        pool->types.data = (VkDescriptorType*)malloc(count * sizeof(VkDescriptorType));

        if(pool->types.data)
        {
            pool->types.size = count;

            for (uint32_t i = 0; i < count; ++i)
                pool->types.data[i] = poolSizes[i].type;

            return true;
        }

        free(pool->types.data);
        pool->types.data = VK_NULL_HANDLE;
    }

    return false;
}


bool DescriptorPool_allocateDescriptorSets(DescriptorPool* pool, VkDescriptorSet* descriptorSets, const VkDescriptorSetLayout* layouts, VkDevice device)
{
    const VkDescriptorSetAllocateInfo allocateInfo = 
    {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = VK_NULL_HANDLE,
        .descriptorPool     = pool->handle,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts        = layouts
    };

    return (vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets) == VK_SUCCESS);
}


void DescriptorPool_writeCombinedImageSampler(DescriptorPool* pool, const VkDescriptorImageInfo* imageInfo, VkDescriptorSet descriptorSet, uint32_t dstBinding, VkDevice device)
{
    const VkWriteDescriptorSet descriptorWrite = 
    {
        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext            = VK_NULL_HANDLE,
        .dstSet           = descriptorSet,
        .dstBinding       = dstBinding,
        .dstArrayElement  = 0,
        .descriptorCount  = 1,
        .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo       = imageInfo,
        .pBufferInfo      = VK_NULL_HANDLE,
        .pTexelBufferView = VK_NULL_HANDLE
    };

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, VK_NULL_HANDLE);
}


void DescriptorPool_destroy(DescriptorPool* pool, VkDevice device)
{
    vkDestroyDescriptorPool(device, pool->handle, VK_NULL_HANDLE);
    free(pool->types.data);
}