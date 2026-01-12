#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "vulkan_api/presentation/main_view.h"
#include "vulkan_api/pipeline/stages/shader/shader.h"
#include "vulkan_api/pipeline/stages/vertex/vertex_input_state.h"
#include "vulkan_api/pipeline/stages/uniform/descriptor_set_layout.h"


typedef struct
{
    struct
    {
        VkPipelineShaderStageCreateInfo* data;
        uint32_t size;
    } shaderInfo;
    
    VertexInputState                       vertexInputState;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineViewportStateCreateInfo      viewportState;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo   multisampling;
    VkPipelineColorBlendAttachmentState    colorBlending;
    DescriptorSetLayout                    layoutInfo;
} GraphicsPipelineState;


void GraphicsPipelineState_setupShaderStages(GraphicsPipelineState* state, const Shader* shaders, uint32_t count);
void GraphicsPipelineState_setupVertexInput(GraphicsPipelineState* state, const VertexInputStateAttributeType* attributes, uint32_t count);
void GraphicsPipelineState_setupInputAssembler(GraphicsPipelineState* state, const VkPrimitiveTopology primitive);
void GraphicsPipelineState_setupViewport(GraphicsPipelineState* state);
void GraphicsPipelineState_setupRasterization(GraphicsPipelineState* state, VkPolygonMode mode);
void GraphicsPipelineState_setupMultisampling(GraphicsPipelineState* state);
void GraphicsPipelineState_setupColorBlending(GraphicsPipelineState* state, VkBool32 enabled);
void GraphicsPipelineState_setupDescriptorSetLayout(GraphicsPipelineState* state, const DescriptorSetLayout* uniformDescriptorSet);
void GraphicsPipelineState_release(const GraphicsPipelineState* state);


typedef struct
{
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout      layout;
    VkPipeline            handle;
} GraphicsPipeline;


bool GraphicsPipeline_create(GraphicsPipeline* pipeline, const GraphicsPipelineState* state, const MainView* view);
void GraphicsPipeline_destroy(GraphicsPipeline* pipeline, VkDevice device);

#endif // !GRAPHICS_PIPELINE_H