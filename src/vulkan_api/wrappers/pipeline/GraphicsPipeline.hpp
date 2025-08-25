#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP

#include <span>

#include <vulkan/vulkan.h>


class GraphicsPipeline
{
public:
    GraphicsPipeline() noexcept;

    bool create(const class MainView& view, std::span<const class ShaderStage> shaders) noexcept;
    void destroy(VkDevice device) noexcept;

    VkDescriptorSetLayout getDescriptorSetLayout() const noexcept;
    VkPipelineLayout      getLayout() const noexcept;
    VkPipeline            getHandle() const noexcept;

private:
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout      m_layout;
    VkPipeline            m_pipeline;
};

#endif // !GRAPHICS_PIPELINE_HPP