#ifndef SHADER_MODULE_H
#define SHADER_MODULE_H

#include <vulkan/vulkan.h>


typedef struct
{
    VkShaderModule        module;
    VkShaderStageFlagBits stage;
} Shader;


Shader Shader_loadFromFile(VkDevice device, VkShaderStageFlagBits stage, const char* filepath);
VkPipelineShaderStageCreateInfo Shader_getInfo(Shader shader);

#endif // !SHADER_MODULE_H