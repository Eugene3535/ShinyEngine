#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP

#include <span>
#include <memory>

#include <vulkan/vulkan.h>

#include "vulkan_api/wrappers/pipeline/stages/shader/ShaderStage.hpp"
#include "vulkan_api/wrappers/pipeline/stages/vertex/VertexInputState.hpp"
#include "vulkan_api/wrappers/pipeline/stages/uniform/DescriptorSetLayout.hpp"


class GraphicsPipeline
{
public:
    struct State
    {
        State* setupShaderStages(std::span<const ShaderStage> shaders)                   noexcept;
        State* setupVertexInput(std::span<const VertexInputState::Attribute> attributes) noexcept;
        State* setupInputAssembler(const VkPrimitiveTopology primitive)                  noexcept;
        State* setupViewport()                                                           noexcept;
        State* setupRasterization(VkPolygonMode mode)                                    noexcept;
        State* setupMultisampling()                                                      noexcept;
        State* setupColorBlending(VkBool32 enabled)                                      noexcept;
        State* setupDescriptorSetLayout(const DescriptorSetLayout& uniformDescriptorSet) noexcept;

    private:
        std::shared_ptr<void> m_data;
        friend class GraphicsPipeline;
    };

    GraphicsPipeline() noexcept;

    VkResult create(const class MainView& view, const State& state) noexcept;
    void destroy(VkDevice device) noexcept;

    VkDescriptorSetLayout getDescriptorSetLayout() const noexcept;
    VkPipelineLayout      getLayout() const noexcept;
    VkPipeline            getHandle() const noexcept;

private:
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout      m_layout;
    VkPipeline            m_handle;
};

#endif // !GRAPHICS_PIPELINE_HPP