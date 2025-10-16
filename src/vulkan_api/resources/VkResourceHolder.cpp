#include "vulkan_api/resources/VkResourceHolder.hpp"


VkResourceHolder::VkResourceHolder(VkPhysicalDevice GPU, VkDevice device, VkQueue queue, VkCommandPool pool) noexcept:
    m_GPU(GPU),
    m_device(device),
    m_queue(queue),
    m_commandPool(pool)
{

}


void VkResourceHolder::cleanup() noexcept
{
    for(const auto& buffer: m_buffers)
    {
        vkDestroyBuffer(m_device, buffer.handle, nullptr);
        vkFreeMemory(m_device, buffer.memory, nullptr);
    }
}