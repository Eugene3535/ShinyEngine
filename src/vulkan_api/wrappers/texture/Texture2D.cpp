#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "vulkan_api/utils/Helpers.hpp"
#include "vulkan_api/wrappers/texture/Texture2D.hpp"

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


    bool transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkDevice device, VkCommandPool pool, VkQueue queue) noexcept
    {
        if(VkCommandBuffer cmd = vk::beginSingleTimeCommands(device, pool))
        {
            VkImageMemoryBarrier barrier = 
            {
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext               = nullptr,
                .srcAccessMask       = VK_ACCESS_NONE,
                .dstAccessMask       = VK_ACCESS_NONE,
                .oldLayout           = oldLayout,
                .newLayout           = newLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image               = image,
                .subresourceRange    = 
                {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel   = 0,
                    .levelCount     = 1,
                    .baseArrayLayer = 0,
                    .layerCount     = 1
                }
            };

            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_NONE;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else return false; // unsupported transition

            vkCmdPipelineBarrier(
                cmd,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            vk::endSingleTimeCommands(cmd, device, pool, queue);

            return true;
        }

        return false;
    }


    bool copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkDevice device, VkCommandPool pool, VkQueue queue) noexcept
    {
        if(VkCommandBuffer cmd = vk::beginSingleTimeCommands(device, pool))
        {
            VkBufferImageCopy region = 
            {
                .bufferOffset      = 0,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource  = 
                {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel       = 0,
                    .baseArrayLayer = 0,
                    .layerCount     = 1
                },
                .imageOffset = 
                {
                    .x = 0, 
                    .y = 0, 
                    .z = 0
                },
                .imageExtent = 
                {
                    .width  = width,
                    .height = height,
                    .depth  = 1
                }
            };

            vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            vk::endSingleTimeCommands(cmd, device, pool, queue);

            return true;
        }

        return false;
    }


    VkResult createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkPhysicalDevice GPU, VkDevice device) noexcept
    {
        VkResult result = VK_SUCCESS;

        VkImageCreateInfo imageInfo = 
        {
            .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext     = nullptr,
            .flags     = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format    = format,
            .extent    = 
            {
                .width  = width,
                .height = height,
                .depth  = 1
            },
            .mipLevels             = 1,
            .arrayLayers           = 1,
            .samples               = VK_SAMPLE_COUNT_1_BIT,
            .tiling                = tiling,
            .usage                 = usage,
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
        };

        if (result = vkCreateImage(device, &imageInfo, nullptr, &image); result == VK_SUCCESS)
        {
            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(device, image, &memRequirements);

            VkMemoryAllocateInfo allocInfo = 
            {
                .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .pNext           = nullptr,
                .allocationSize  = memRequirements.size,
                .memoryTypeIndex = vk::findMemoryType(memRequirements.memoryTypeBits, properties, GPU)
            };

            if (result = vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory); result == VK_SUCCESS)
                result = vkBindImageMemory(device, image, imageMemory, 0);
        }

        return result;
    }
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

    if(createImage(
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
        
    if (!transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device, pool, queue))
        return false;

    if (!copyBufferToImage(stagingBuffer, m_image, static_cast<uint32_t>(stbImage.width), static_cast<uint32_t>(stbImage.height), device, pool, queue))
        return false;

    if (!transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device, pool, queue))
        return false;

    if (createImageView(device) != VK_SUCCESS)
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


VkResult Texture2D::createImageView(VkDevice device) noexcept
{
    VkImageViewCreateInfo viewInfo = 
    {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = 0,
        .image            = m_image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = VK_FORMAT_R8G8B8A8_SRGB,
        .components       = 
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = 
        {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };

    return vkCreateImageView(device, &viewInfo, nullptr, &m_imageView);
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