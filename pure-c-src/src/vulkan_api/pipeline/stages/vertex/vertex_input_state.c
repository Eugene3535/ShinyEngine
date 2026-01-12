#include "vulkan_api/pipeline/stages/vertex/vertex_input_state.h"


static uint32_t shader_attribute_type_to_component_count(const VertexInputStateAttributeType type);
static size_t   shader_attribute_type_sizeof(const VertexInputStateAttributeType type);
static VkFormat shader_attribute_type_to_vk_format(const VertexInputStateAttributeType type);



void VertexInputState_create(VertexInputState* state, const VertexInputStateAttributeType* attributes, uint32_t count)
{
    state->attributeDescriptions.data = (VkVertexInputAttributeDescription*)malloc(count * sizeof(VkVertexInputAttributeDescription));

    if(state->attributeDescriptions.data)
    {
        state->attributeDescriptions.size = count;

        VkVertexInputAttributeDescription* descriptions = state->attributeDescriptions.data;
        uint32_t offset = 0;

        for (uint32_t i = 0; i < count; ++i)
        {
            descriptions[i].location = i;
            descriptions[i].binding  = 0;
            descriptions[i].format   = shader_attribute_type_to_vk_format(attributes[i]);
            descriptions[i].offset   = offset;

            offset += shader_attribute_type_sizeof(attributes[i]);
        }

        state->bindingDescription.binding   = 0;
        state->bindingDescription.stride    = offset;
        state->bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }
}


VkPipelineVertexInputStateCreateInfo VertexInputState_getInfo(const VertexInputState* state)
{
    VkPipelineVertexInputStateCreateInfo info = 
    {
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                           = VK_NULL_HANDLE,
        .flags                           = 0,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &state->bindingDescription,
        .vertexAttributeDescriptionCount = state->attributeDescriptions.size,
        .pVertexAttributeDescriptions    = state->attributeDescriptions.data
    };

    return info;
}



uint32_t shader_attribute_type_to_component_count(const VertexInputStateAttributeType type)
{
    switch (type)
    {
        case Float1:
        case Int1:
            return 1;

        case Float2:
        case Int2:
            return 2;

        case Float3:
        case Int3:
            return 3;

        case Float4:
        case Int4:
            return 4;
    }

    return 0;
}


size_t shader_attribute_type_sizeof(const VertexInputStateAttributeType type)
{
    switch (type)
    {
        case Float1:
        case Float2:
        case Float3:
        case Float4:
            return sizeof(float) * shader_attribute_type_to_component_count(type);

        case Int1:
        case Int2:
        case Int3:
        case Int4:
            return sizeof(int32_t) * shader_attribute_type_to_component_count(type);
    }

    return 0;
}


VkFormat shader_attribute_type_to_vk_format(const VertexInputStateAttributeType type)
{
    switch (type)
    {
        case Float1: return VK_FORMAT_R32_SFLOAT;
        case Float2: return VK_FORMAT_R32G32_SFLOAT;
        case Float3: return VK_FORMAT_R32G32B32_SFLOAT;
        case Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;

        case Int1: return VK_FORMAT_R32_SINT;
        case Int2: return VK_FORMAT_R32G32_SINT;
        case Int3: return VK_FORMAT_R32G32B32_SINT;
        case Int4: return VK_FORMAT_R32G32B32A32_SINT;
    }

    return VK_FORMAT_UNDEFINED;
}