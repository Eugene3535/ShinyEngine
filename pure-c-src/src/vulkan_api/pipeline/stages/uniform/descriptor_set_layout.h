#ifndef DESCRIPTOR_SET_LAYOUT_H
#define DESCRIPTOR_SET_LAYOUT_H

#include <stdint.h>

#include <vulkan/vulkan.h>


typedef struct
{
    VkDescriptorSetLayoutBinding* bindings;
    uint32_t size;
} DescriptorSetLayout;


void DescriptorSetLayout_addDescriptor(DescriptorSetLayout* layout, VkDescriptorType type, VkShaderStageFlagBits shaderStage);
VkDescriptorSetLayoutCreateInfo DescriptorSetLayout_getInfo(const DescriptorSetLayout* layout);


#endif // !DESCRIPTOR_SET_LAYOUT_H