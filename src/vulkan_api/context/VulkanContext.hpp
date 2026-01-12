#ifndef VULKAN_CONTEXT_HPP
#define VULKAN_CONTEXT_HPP

#include <vulkan/vulkan.h>


class VulkanContext
{
public:
    VulkanContext() noexcept;
    ~VulkanContext();

    VkResult initialize() noexcept;
    void destroy() noexcept;

    VkInstance       getInstance()             const noexcept;
    VkPhysicalDevice getPhysicalDevice()       const noexcept;
    VkDevice         getDevice()               const noexcept;
    VkQueue          getQueue()                const noexcept;
    uint32_t         getMainQueueFamilyIndex() const noexcept;

private:
    VkResult createInstance()  noexcept;
    VkResult selectVideoCard() noexcept;
    VkResult createDevice()    noexcept;

    VkInstance       m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice         m_device;
    VkQueue          m_queue;
    uint32_t         m_mainQueueFamilyIndex;
};

#endif // !VULKAN_CONTEXT_HPP