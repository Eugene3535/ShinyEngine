#include "vulkan_api/pipeline/stages/vertex/VertexInputState.hpp"

namespace
{
    constexpr uint32_t shader_attribute_type_to_component_count(const VertexInputState::Attribute::Type type) noexcept
    {
        switch (type)
        {
            case VertexInputState::Attribute::Type::Float:
            case VertexInputState::Attribute::Type::Int:
                return 1;

            case VertexInputState::Attribute::Type::Float2:
            case VertexInputState::Attribute::Type::Int2:
                return 2;

            case VertexInputState::Attribute::Type::Float3:
            case VertexInputState::Attribute::Type::Int3:
                return 3;

            case VertexInputState::Attribute::Type::Float4:
            case VertexInputState::Attribute::Type::Int4:
                return 4;
        }

        return 0;
    }


    constexpr size_t shader_attribute_type_sizeof(const VertexInputState::Attribute::Type type) noexcept
    {
        switch (type)
        {
            case VertexInputState::Attribute::Type::Float:
            case VertexInputState::Attribute::Type::Float2:
            case VertexInputState::Attribute::Type::Float3:
            case VertexInputState::Attribute::Type::Float4:
                return sizeof(float) * shader_attribute_type_to_component_count(type);

            case VertexInputState::Attribute::Type::Int:
            case VertexInputState::Attribute::Type::Int2:
            case VertexInputState::Attribute::Type::Int3:
            case VertexInputState::Attribute::Type::Int4:
                return sizeof(int32_t) * shader_attribute_type_to_component_count(type);
        }

        return 0;
    }


    constexpr VkFormat shader_attribute_type_to_vk_format(const VertexInputState::Attribute::Type type) noexcept
    {
        switch (type)
        {
            case VertexInputState::Attribute::Type::Float:  return VK_FORMAT_R32_SFLOAT;
            case VertexInputState::Attribute::Type::Float2: return VK_FORMAT_R32G32_SFLOAT;
            case VertexInputState::Attribute::Type::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
            case VertexInputState::Attribute::Type::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;

            case VertexInputState::Attribute::Type::Int:  return VK_FORMAT_R32_SINT;
            case VertexInputState::Attribute::Type::Int2: return VK_FORMAT_R32G32_SINT;
            case VertexInputState::Attribute::Type::Int3: return VK_FORMAT_R32G32B32_SINT;
            case VertexInputState::Attribute::Type::Int4: return VK_FORMAT_R32G32B32A32_SINT;
        }

        return VK_FORMAT_UNDEFINED;
    }
}



VertexInputState::Attribute::Attribute(const Type attrType) noexcept:
    type(attrType),
    sizeInBytes(shader_attribute_type_sizeof(attrType))
{

}


VertexInputState::VertexInputState(std::span<const VertexInputState::Attribute> attributes) noexcept
{
    m_attributeDescription.resize(attributes.size());
    uint32_t offset = 0;

    for (uint32_t i = 0; i < m_attributeDescription.size(); ++i)
    {
        m_attributeDescription[i].location = i;
        m_attributeDescription[i].binding = 0;
        m_attributeDescription[i].format = shader_attribute_type_to_vk_format(attributes[i].type);
        m_attributeDescription[i].offset = offset;

        uint32_t sizeInBytes = shader_attribute_type_sizeof(attributes[i].type);
        offset += sizeInBytes;
    }

    m_bindingDescription.binding = 0;
    m_bindingDescription.stride = offset;
    m_bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}


VkPipelineVertexInputStateCreateInfo VertexInputState::getinfo() const noexcept
{
    return
    {
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                           = nullptr,
        .flags                           = 0,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &m_bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescription.size()),
        .pVertexAttributeDescriptions    = m_attributeDescription.data()
    };
}