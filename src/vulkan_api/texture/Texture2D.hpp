#ifndef TEXTURE2D_HPP
#define TEXTURE2D_HPP

#include <vulkan/vulkan.h>

class Texture2D
{
public:
    Texture2D() noexcept;

    bool loadFromFile(const char* filepath, VkPhysicalDevice GPU, VkDevice device, VkCommandPool pool, VkQueue queue) noexcept;
    void destroy(VkDevice device) noexcept;

    VkImageView getImageView() const noexcept;
    VkSampler   getSampler() const noexcept;

private:
    VkResult createSampler(VkPhysicalDevice GPU, VkDevice device) noexcept;

    VkDeviceMemory m_imageMemory;
    VkImage        m_image;
    VkImageView    m_imageView;
    VkSampler      m_sampler;
};

#endif // !TEXTURE2D_HPP