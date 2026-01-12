#include <string.h>

#include <cglm/struct/mat4.h>

#include "vulkan_api/utils/tools.h"
#include "vulkan_api/presentation/main_view.h"
#include "vulkan_api/pipeline/graphics_pipeline.h"



void GraphicsPipelineState_setupShaderStages(GraphicsPipelineState* state, const Shader* shaders, uint32_t count)
{
    state->shaderInfo.data = (VkPipelineShaderStageCreateInfo*)malloc(count * sizeof(VkPipelineShaderStageCreateInfo));
    state->shaderInfo.size = count;

    if(state->shaderInfo.data)
    {
        VkPipelineShaderStageCreateInfo* data = state->shaderInfo.data;

        for (uint32_t i = 0; i < count; ++i)
        {
            VkPipelineShaderStageCreateInfo info = Shader_getInfo(shaders[i]);
            memcpy(data + i, &info, sizeof(VkPipelineShaderStageCreateInfo));
        }
    } 
}


void GraphicsPipelineState_setupVertexInput(GraphicsPipelineState* state, const VertexInputStateAttributeType* attributes, uint32_t count)
{
    VertexInputState_create(&state->vertexInputState, attributes, count); // TODO сюда перенести инициализацию входного состояния вершин 
}


void GraphicsPipelineState_setupInputAssembler(GraphicsPipelineState* state, const VkPrimitiveTopology primitive)
{
    state->inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    state->inputAssembly.pNext                  = VK_NULL_HANDLE;
    state->inputAssembly.flags                  = 0;
    state->inputAssembly.topology               = primitive;
    state->inputAssembly.primitiveRestartEnable = VK_FALSE;
}


void GraphicsPipelineState_setupViewport(GraphicsPipelineState* state)
{
    state->viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    state->viewportState.pNext         = VK_NULL_HANDLE;
    state->viewportState.flags         = 0;
    state->viewportState.viewportCount = 1;
    state->viewportState.pViewports    = VK_NULL_HANDLE;
    state->viewportState.scissorCount  = 1;
    state->viewportState.pScissors     = VK_NULL_HANDLE;
}


void GraphicsPipelineState_setupRasterization(GraphicsPipelineState* state, VkPolygonMode mode)
{
    state->rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    state->rasterizer.pNext                   = VK_NULL_HANDLE;
    state->rasterizer.flags                   = 0;
    state->rasterizer.depthClampEnable        = VK_FALSE;
    state->rasterizer.rasterizerDiscardEnable = VK_FALSE;
    state->rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    state->rasterizer.cullMode                = VK_CULL_MODE_NONE; // TODO Добавить переключение отсечения граней, например VK_CULL_MODE_FRONT_BIT
    state->rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    state->rasterizer.depthBiasEnable         = VK_FALSE;
    state->rasterizer.depthBiasConstantFactor = 0.f;
    state->rasterizer.depthBiasClamp          = 0.f;
    state->rasterizer.depthBiasSlopeFactor    = 0.f;
    state->rasterizer.lineWidth               = 1.f;
}


void GraphicsPipelineState_setupMultisampling(GraphicsPipelineState* state)
{
    state->multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    state->multisampling.pNext                 = VK_NULL_HANDLE;
    state->multisampling.flags                 = 0;
    state->multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    state->multisampling.sampleShadingEnable   = VK_FALSE;
    state->multisampling.minSampleShading      = 1.f;
    state->multisampling.pSampleMask           = VK_NULL_HANDLE;
    state->multisampling.alphaToCoverageEnable = VK_FALSE;
    state->multisampling.alphaToOneEnable      = VK_FALSE;
}


void GraphicsPipelineState_setupColorBlending(GraphicsPipelineState* state, VkBool32 enabled)
{
    state->colorBlending.blendEnable         = enabled;
    state->colorBlending.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    state->colorBlending.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    state->colorBlending.colorBlendOp        = VK_BLEND_OP_ADD;
    state->colorBlending.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    state->colorBlending.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    state->colorBlending.alphaBlendOp        = VK_BLEND_OP_ADD;
    state->colorBlending.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
}


void GraphicsPipelineState_setupDescriptorSetLayout(GraphicsPipelineState* state, const DescriptorSetLayout* uniformDescriptorSet)
{
    memcpy(&state->layoutInfo, uniformDescriptorSet, sizeof(DescriptorSetLayout));
}


void GraphicsPipelineState_release(const GraphicsPipelineState* state)
{
    free(state->shaderInfo.data);
    free(state->vertexInputState.attributeDescriptions.data);
}



bool GraphicsPipeline_create(GraphicsPipeline* pipeline, const GraphicsPipelineState* state, const MainView* view)
{
    VkPhysicalDevice GPU    = view->context->GPU;
    VkDevice         device = view->context->device;
    GraphicsPipeline_destroy(pipeline, device); // for recreate case

    const VkFormat colorFormat = view->format;
    const VkFormat depthFormat = find_depth_format(GPU);

    const VkPipelineVertexInputStateCreateInfo vertexInput = VertexInputState_getInfo(&state->vertexInputState);

    const VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo =
    {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .pNext                   = VK_NULL_HANDLE,
        .viewMask                = 0,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &colorFormat,
        .depthAttachmentFormat   = depthFormat,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    const VkPipelineColorBlendStateCreateInfo colorBlending =
    {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext           = VK_NULL_HANDLE,
        .flags           = 0,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments    = &state->colorBlending
    };

    const VkDynamicState dynamicStates[] = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    const VkPipelineDynamicStateCreateInfo dynamicState = 
    {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext             = VK_NULL_HANDLE,
        .flags             = 0,
        .dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState),
        .pDynamicStates    = dynamicStates
    };

    const VkDescriptorSetLayoutCreateInfo layoutInfo = DescriptorSetLayout_getInfo(&state->layoutInfo);

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, VK_NULL_HANDLE, &pipeline->descriptorSetLayout) != VK_SUCCESS)
        return false;

    const VkPushConstantRange pushConstantRange = 
    {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset     = 0,
        .size       = sizeof(mat4s)
    };

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo = 
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = VK_NULL_HANDLE,
        .flags                  = 0,
        .setLayoutCount         = 1,
        .pSetLayouts            = &pipeline->descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &pushConstantRange
    };

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, VK_NULL_HANDLE, &pipeline->layout) != VK_SUCCESS)
        return false;

    const VkPipelineDepthStencilStateCreateInfo depthStencil = 
    {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext                 = VK_NULL_HANDLE,
        .flags                 = 0,
        .depthTestEnable       = VK_TRUE,
        .depthWriteEnable      = VK_TRUE,
        .depthCompareOp        = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
        .front                 = {0},
        .back                  = {0},
        .minDepthBounds        = 0.f,
        .maxDepthBounds        = 1.f
    };

    const VkGraphicsPipelineCreateInfo pipelineInfo = 
    {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &pipelineRenderingInfo,
        .flags               = 0,
        .stageCount          = state->shaderInfo.size,
        .pStages             = state->shaderInfo.data,
        .pVertexInputState   = &vertexInput,
        .pInputAssemblyState = &state->inputAssembly,
        .pTessellationState  = VK_NULL_HANDLE,
        .pViewportState      = &state->viewportState,
        .pRasterizationState = &state->rasterizer,
        .pMultisampleState   = &state->multisampling,
        .pDepthStencilState  = &depthStencil,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = pipeline->layout,
        .renderPass          = VK_NULL_HANDLE,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = 0
    };

    return (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &pipeline->handle) == VK_SUCCESS);
}


void GraphicsPipeline_destroy(GraphicsPipeline* pipeline, VkDevice device)
{
    if(pipeline->handle)
    {
        vkDestroyPipeline(device, pipeline->handle, VK_NULL_HANDLE);
        vkDestroyPipelineLayout(device, pipeline->layout, VK_NULL_HANDLE);
        vkDestroyDescriptorSetLayout(device, pipeline->descriptorSetLayout, VK_NULL_HANDLE);

        pipeline->handle              = VK_NULL_HANDLE;
        pipeline->layout              = VK_NULL_HANDLE;
        pipeline->descriptorSetLayout = VK_NULL_HANDLE;
    }
}