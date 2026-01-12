#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "vulkan_api/utils/Helpers.hpp"
#include "vulkan_api/texture/Texture2D.hpp"

namespace
{
    struct StbImage
    {
        StbImage(const char* filepath, int mode) noexcept:
            pixels(nullptr), width(0), height(0), channels(0)
        {
            pixels = stbi_load(filepath, &width, &height, &channels, mode);
        }

        ~StbImage()
        {
            stbi_image_free(pixels);
        }

        stbi_uc* pixels;
        int32_t  width;
        int32_t  height;
        int32_t  channels;
    };

    struct BufferMemoryDeleter
    {
        ~BufferMemoryDeleter() 
        {
            if (buffer) vkDestroyBuffer(device, buffer, nullptr);
            if (memory) vkFreeMemory(device, memory, nullptr);
        }

        VkDeviceMemory memory = nullptr;
        VkBuffer buffer = nullptr;
        VkDevice device = nullptr;
    };
}



Texture2D::Texture2D() noexcept:
    m_imageMemory(nullptr),
    m_image(nullptr),
    m_imageView(nullptr),
    m_sampler(nullptr)
{

}


bool Texture2D::loadFromFile(const char* filepath, VkPhysicalDevice GPU, VkDevice device, VkCommandPool pool, VkQueue queue) noexcept
{
    StbImage stbImage(filepath, STBI_rgb_alpha);

    if ( ! stbImage.pixels )
        return false;

    VkDeviceSize imageSize = stbImage.width * stbImage.height * 4;
    
    VkDeviceMemory stagingBufferMemory;
    VkBuffer stagingBuffer = vk::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory, device, GPU);

    if(!stagingBuffer)
        return false;

    BufferMemoryDeleter guard = { stagingBufferMemory, stagingBuffer, device };

    if (void* data; vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data) == VK_SUCCESS)
    {
        memcpy(data, stbImage.pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);
    }
    else return false;

    if(vk::createImage2D(
        stbImage.width, 
        stbImage.height, 
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        m_image, 
        m_imageMemory, 
        GPU, 
        device) != VK_SUCCESS)
        return false;
        
    if ( ! vk::transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device, pool, queue))
        return false;

    if ( ! vk::copyBufferToImage(stagingBuffer, m_image, static_cast<uint32_t>(stbImage.width), static_cast<uint32_t>(stbImage.height), device, pool, queue))
        return false;

    if ( ! vk::transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device, pool, queue))
        return false;

    if(vk::createImageView2D(device, m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_imageView) != VK_SUCCESS)
        return false;
    
    if (createSampler(GPU, device) != VK_SUCCESS)
        return false;
    
    return true;
}


VkImageView Texture2D::getImageView() const noexcept
{
    return m_imageView;
}


VkSampler Texture2D::getSampler() const noexcept
{
    return m_sampler;
}


void Texture2D::destroy(VkDevice device) noexcept
{
    vkDestroySampler(device, m_sampler, nullptr);
    vkDestroyImageView(device, m_imageView, nullptr);
    vkDestroyImage(device, m_image, nullptr);
    vkFreeMemory(device, m_imageMemory, nullptr);
}


VkResult Texture2D::createSampler(VkPhysicalDevice GPU, VkDevice device) noexcept
{
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(GPU, &properties);

    VkSamplerCreateInfo samplerInfo = 
    {
        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext                   = nullptr,
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

    return vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler);
}