#include <stdlib.h>
#include <string.h>

#include "vulkan_api/pipeline/stages/uniform/descriptor_set_layout.h"


void DescriptorSetLayout_addDescriptor(DescriptorSetLayout* layout, VkDescriptorType type, VkShaderStageFlagBits shaderStage)
{
    const uint32_t binding = layout->size;
    const VkShaderStageFlags flags = shaderStage;

    VkDescriptorSetLayoutBinding* reallocatedBindings = (VkDescriptorSetLayoutBinding*)realloc(layout->bindings, (binding + 1) * sizeof(VkDescriptorSetLayoutBinding));
    
    if(reallocatedBindings)
    {
        layout->bindings = reallocatedBindings;
        layout->size++;

        const VkDescriptorSetLayoutBinding nextBinding = 
        {
            .binding            = binding,
            .descriptorType     = type,
            .descriptorCount    = 1,
            .stageFlags         = flags,
            .pImmutableSamplers = VK_NULL_HANDLE
        };

        memcpy(reallocatedBindings + binding, &nextBinding, sizeof(VkDescriptorSetLayoutBinding));
    }
}


VkDescriptorSetLayoutCreateInfo DescriptorSetLayout_getInfo(const DescriptorSetLayout* layout)
{
    const VkDescriptorSetLayoutCreateInfo info = 
    {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = VK_NULL_HANDLE,
        .flags        = 0,
        .bindingCount = layout->size,
        .pBindings    = layout->bindings
    };

    return info;
}
