#ifndef VERTEX_INPUT_STATE_HPP
#define VERTEX_INPUT_STATE_HPP

#include <cstdint>
#include <span>
#include <vector>

#include <vulkan/vulkan.h>


class VertexInputState final
{
public:
    class Attribute final
    {
    public:
        enum Type
        {
            Float,
            Float2,
            Float3,
            Float4,
            Int,
            Int2,
            Int3,
            Int4
        };

        Attribute(const Type attrType) noexcept;

        Type   type;
        size_t sizeInBytes;
    };

    VertexInputState(std::span<const Attribute> attributes) noexcept;

    VkPipelineVertexInputStateCreateInfo getinfo() const noexcept;

private:
    std::vector<VkVertexInputAttributeDescription> m_attributeDescription;
    VkVertexInputBindingDescription m_bindingDescription;
};

#endif // !VERTEX_INPUT_STATE_HPP