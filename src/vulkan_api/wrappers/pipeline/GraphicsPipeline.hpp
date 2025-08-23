#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP

#include <span>


class GraphicsPipeline
{
public:
    GraphicsPipeline() noexcept;

    bool create(const class MainView& view, std::span<const class ShaderStage> shaders) noexcept;
    void destroy() noexcept;

    bool writeCommandBuffer(struct VkCommandBuffer_T* commandBuffer, uint32_t currentFrame, uint32_t imageIndex, const class Mesh& mesh, struct VkDescriptorSet_T* descriptorSet) noexcept;

//private:
    struct VkDescriptorSetLayout_T* m_descriptorSetLayout;
    struct VkPipelineLayout_T*      m_layout;
    struct VkPipeline_T*            m_pipeline;
    const class MainView*           m_mainView;
};

#endif // !GRAPHICS_PIPELINE_HPP