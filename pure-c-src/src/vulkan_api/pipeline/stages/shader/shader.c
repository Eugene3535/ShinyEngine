#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vulkan_api/pipeline/stages/shader/shader.h"

// Helper function to read a file into a buffer
static size_t read_shader_file(const char* filename, char** buffer);

// Function to create a shader module from SPIR-V data
static VkShaderModule create_shader_module(VkDevice device, const char* filename);


Shader Shader_loadFromFile(VkDevice device, VkShaderStageFlagBits stage, const char* filepath)
{
    Shader shader = {0};
    VkShaderModule shaderModule = create_shader_module(device, filepath);

    if(shaderModule)
    {
        shader.module = shaderModule;
        shader.stage = stage;
    }

    return shader;
}


VkPipelineShaderStageCreateInfo Shader_getInfo(Shader shader)
{
    const VkPipelineShaderStageCreateInfo info =
    {
        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = VK_NULL_HANDLE,
        .flags               = 0,
        .stage               = shader.stage,
        .module              = shader.module,
        .pName               = "main",
        .pSpecializationInfo = VK_NULL_HANDLE
    };

    return info;
}


size_t read_shader_file(const char* filename, char** buffer) 
{
    FILE* file = fopen(filename, "rb");

    if (!file) 
    {
#ifdef DEBUG
        fprintf(stderr, "Failed to open file: %s\n", filename);
#endif
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size_t fileSize = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = (char*)malloc(fileSize);

    if (!*buffer) 
    {
        fclose(file);
#ifdef DEBUG
        fprintf(stderr, "Failed to allocate memory for shader file: %s\n", filename);
#endif
        return 0;
    }

    size_t bytesRead = fread(*buffer, 1, fileSize, file);

    if (bytesRead != fileSize) 
    {
        free(*buffer);
        fclose(file);
#ifdef DEBUG
        fprintf(stderr, "Failed to read full file: %s\n", filename);
#endif
        return 0;
    }

    fclose(file);

    return fileSize;
}


VkShaderModule create_shader_module(VkDevice device, const char* filename) 
{
    char* code;
    size_t codeSize = read_shader_file(filename, &code);

    if (codeSize == 0)
        return VK_NULL_HANDLE;

    VkShaderModuleCreateInfo createInfo = 
    {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = VK_NULL_HANDLE,
        .flags    = 0,
        .codeSize = codeSize,             // The pointer must be a uint32_t pointer and aligned correctly.
        .pCode    = (const uint32_t*)code // Ensure your memory allocation is suitable for this (malloc usually is).
    };

    VkShaderModule shaderModule;

    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) 
    {
#ifdef DEBUG
        fprintf(stderr, "Failed to create shader module from file: %s\n", filename);
#endif
        shaderModule = VK_NULL_HANDLE;
    }

    free(code);

    return shaderModule;
}