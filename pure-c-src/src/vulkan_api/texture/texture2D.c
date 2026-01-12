#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "vulkan_api/utils/tools.h"
#include "vulkan_api/context/context.h"
#include "vulkan_api/texture/texture2D.h"


static bool create_sampler(Texture2D* texture, VkPhysicalDevice gpu, VkDevice device);


bool Texture2D_loadFromFile(Texture2D* texture, const char* filepath, const void* ctx, VkCommandPool pool)
{
    bool result = false;
    const VulkanContext* context = (const VulkanContext*)ctx;

    int32_t width, height, channels;
    stbi_uc* pixels = stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha);

    if(!pixels)
        return false;

    VkDeviceSize imageSize = width * height * 4;

    VkDeviceMemory stagingBufferMemory;
    VkBuffer stagingBuffer = create_buffer(
                                            imageSize, 
                                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                                            &stagingBufferMemory, 
                                            context->device, 
                                            context->GPU);

    if(!stagingBuffer)
        return false;

    {
        void* data;

        if(vkMapMemory(context->device, stagingBufferMemory, 0, imageSize, 0, &data) == VK_SUCCESS)
        {
            memcpy(data, pixels, imageSize);
            vkUnmapMemory(context->device, stagingBufferMemory);
        }
        else goto error_create_texture;
    }

    VkExtent2D extent = { width, height };

    if(!create_image_2D(
                        extent, 
                        VK_FORMAT_R8G8B8A8_SRGB, 
                        VK_IMAGE_TILING_OPTIMAL, 
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                        &texture->image, 
                        &texture->imageMemory, 
                        context->GPU, 
                        context->device))
        goto error_create_texture;
        
    if ( ! transition_image_layout(texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, context->device, pool, context->queue) )
        goto error_create_texture;

    if ( ! copy_buffer_to_image(stagingBuffer, texture->image, width, height, context->device, pool, context->queue) )
        goto error_create_texture;

    if ( ! transition_image_layout(texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, context->device, pool, context->queue) )
        goto error_create_texture;

    if( ! create_image_view_2D(context->device, texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, &texture->imageView))
        goto error_create_texture;
    
    if ( ! create_sampler(texture, context->GPU, context->device))
        goto error_create_texture;

    result = true;

error_create_texture:
    vkDestroyBuffer(context->device, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(context->device, stagingBufferMemory, VK_NULL_HANDLE);
    stbi_image_free(pixels);

    return result;
}


void Texture2D_destroy(Texture2D* texture, VkDevice device)
{
    vkDestroySampler(device, texture->sampler, VK_NULL_HANDLE);
    vkDestroyImageView(device, texture->imageView, VK_NULL_HANDLE);
    vkDestroyImage(device, texture->image, VK_NULL_HANDLE);
    vkFreeMemory(device, texture->imageMemory, VK_NULL_HANDLE);
}


bool create_sampler(Texture2D* texture, VkPhysicalDevice gpu, VkDevice device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(gpu, &properties);

    const VkSamplerCreateInfo samplerInfo = 
    {
        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext                   = VK_NULL_HANDLE,
        .flags                   = 0,
        .magFilter               = VK_FILTER_LINEAR,
        .minFilter               = VK_FILTER_LINEAR,
        .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias              = 0.f,
        .anisotropyEnable        = VK_TRUE,
        .maxAnisotropy           = properties.limits.maxSamplerAnisotropy,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_ALWAYS,
        .minLod                  = 0.f,
        .maxLod                  = 0.f,
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    return (vkCreateSampler(device, &samplerInfo, VK_NULL_HANDLE, &texture->sampler) == VK_SUCCESS);
}