#ifndef VERTEX_INPUT_STATE_H
#define VERTEX_INPUT_STATE_H

#include <vulkan/vulkan.h>


typedef enum
{
    Float1,
    Float2,
    Float3,
    Float4,
    Int1,
    Int2,
    Int3,
    Int4
} VertexInputStateAttributeType;


typedef struct
{
    struct
    {
        VkVertexInputAttributeDescription* data;
        uint32_t size;
    } attributeDescriptions;

    VkVertexInputBindingDescription bindingDescription;
} VertexInputState;


void VertexInputState_create(VertexInputState* state, const VertexInputStateAttributeType* attributes, uint32_t count);
VkPipelineVertexInputStateCreateInfo VertexInputState_getInfo(const VertexInputState* state);

#endif // !VERTEX_INPUT_STATE_H