#include <array>

#include "vulkan_api/wrappers/pipeline/stages/shader/ShaderStage.hpp"
#include "vulkan_api/wrappers/pipeline/stages/vertex/VertexInputState.hpp"
#include "vulkan_api/wrappers/pipeline/stages/uniform/DescriptorSetLayout.hpp"
#include "vulkan_api/wrappers/view/MainView.hpp"
#include "vulkan_api/wrappers/mesh/Mesh.hpp"
#include "vulkan_api/utils/Structures.hpp"
#include "vulkan_api/wrappers/pipeline/GraphicsPipeline.hpp"


namespace
{
    VkPipelineInputAssemblyStateCreateInfo create_input_assembly_stage(VkPrimitiveTopology primitive) noexcept
    {
        return
        {    
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext                  = nullptr,
            .flags                  = 0,
            .topology               = primitive,
            .primitiveRestartEnable = VK_FALSE
        };
    }


    VkPipelineViewportStateCreateInfo create_viewport_stage() noexcept
    {
        return 
        {
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext         = nullptr,
            .flags         = 0,
            .viewportCount = 1,
            .pViewports    = nullptr,
            .scissorCount  = 1,
            .pScissors     = nullptr
        };
    }


    VkPipelineRasterizationStateCreateInfo create_rasterization_stage(VkPolygonMode mode) noexcept
    {
        return 
        {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0,
            .depthClampEnable        = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = mode,
            .cullMode                = VK_CULL_MODE_FRONT_BIT,
            .frontFace               = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable         = VK_FALSE,
            .depthBiasConstantFactor = 0.f,
            .depthBiasClamp          = 0.f,
            .depthBiasSlopeFactor    = 0.f,
            .lineWidth               = 1.f
        };
    }


    VkPipelineMultisampleStateCreateInfo create_multisampling_stage()
    {
        return
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };
    }

    VkPipelineColorBlendAttachmentState create_color_blend_attachment(bool enabled)
    {
        return
        {
            .blendEnable         = enabled ? VK_TRUE : VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp        = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .alphaBlendOp        = VK_BLEND_OP_ADD,
            .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };
    }
}


GraphicsPipeline::GraphicsPipeline() noexcept:
    m_descriptorSetLayout(nullptr),
    m_layout(nullptr),
    m_pipeline(nullptr)
{

}


bool GraphicsPipeline::create(const MainView& view, std::span<const ShaderStage> shaders) noexcept
{
    auto device = view.getVulkanApi()->getDevice();
    destroy(device);

    const VkFormat format = view.getFormat();

    const VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo =
    {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &format
    };

    const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = 
    {
        shaders[0].getInfo(), 
        shaders[1].getInfo()
    };

    std::array<const VertexInputState::Attribute, 3> attributes =
    {
        VertexInputState::Attribute::Float2,
        VertexInputState::Attribute::Float3,
        VertexInputState::Attribute::Float2
    };

    auto vertex_input_state = std::make_unique<VertexInputState>(attributes);
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = vertex_input_state->getinfo();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = create_input_assembly_stage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    VkPipelineViewportStateCreateInfo      viewportState = create_viewport_stage();
    VkPipelineRasterizationStateCreateInfo rasterizer    = create_rasterization_stage(VK_POLYGON_MODE_FILL);
    VkPipelineMultisampleStateCreateInfo   multisampling = create_multisampling_stage();

    VkPipelineColorBlendAttachmentState colorBlendAttachment = create_color_blend_attachment(false);
    VkPipelineColorBlendStateCreateInfo colorBlending
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    std::array<VkDynamicState, 2> dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = 
    {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext             = nullptr,
        .flags             = 0,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates    = dynamicStates.data()
    };

    {// Descriptor Set Layout
        DescriptorSetLayout uniformDescriptors;
        uniformDescriptors.addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
        uniformDescriptors.addDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

        const VkDescriptorSetLayoutCreateInfo layoutInfo = uniformDescriptors.getInfo();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
            return false;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = 
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .setLayoutCount         = 1,
        .pSetLayouts            = &m_descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = nullptr
    };

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_layout) != VK_SUCCESS)
        return false;

    VkGraphicsPipelineCreateInfo pipelineInfo = 
    {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &pipelineRenderingInfo,
        .flags               = 0,
        .stageCount          = static_cast<uint32_t>(shaders.size()),
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pTessellationState  = nullptr,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = nullptr,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = m_layout,
        .renderPass          = nullptr,
        .subpass             = 0,
        .basePipelineHandle  = nullptr,
        .basePipelineIndex   = 0
    };

    return (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) == VK_SUCCESS);
}


void GraphicsPipeline::destroy(VkDevice device) noexcept
{
    if(m_pipeline)
    {
        vkDestroyPipeline(device, m_pipeline, nullptr);
        vkDestroyPipelineLayout(device, m_layout, nullptr);
        vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
    }
}


VkDescriptorSetLayout GraphicsPipeline::getDescriptorSetLayout() const noexcept
{
    return m_descriptorSetLayout;
}


VkPipelineLayout GraphicsPipeline::getLayout() const noexcept
{
    return m_layout;
}


VkPipeline GraphicsPipeline::getHandle() const noexcept
{
    return m_pipeline;
}