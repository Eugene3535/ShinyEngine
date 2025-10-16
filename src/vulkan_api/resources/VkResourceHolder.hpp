#ifndef VK_RESOURCE_HOLDER_HPP
#define VK_RESOURCE_HOLDER_HPP

#include <vector>
#include <span>

#include "vulkan_api/utils/Helpers.hpp"


struct Buffer
{
    VkBuffer handle = nullptr;
    uint32_t size = 0;
};


class VkResourceHolder
{
public:
    VkResourceHolder(VkPhysicalDevice GPU, VkDevice device, VkQueue queue, VkCommandPool pool) noexcept;

    template <class T>
    Buffer createBuffer(std::span<const T> rawData, VkBufferUsageFlagBits flag) noexcept
    {
        BufferData bufferData;
        bufferData.size = rawData.size();
        const VkDeviceSize bufferSize = sizeof(T) * rawData.size();

        VkDeviceMemory stagingBufferMemory;
        VkBuffer stagingBuffer = vk::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory, m_device, m_GPU);

        if(!stagingBuffer)
            return {};

        struct BufferMemoryDeleter
        {
            ~BufferMemoryDeleter() 
            {
                vkDestroyBuffer(device, buffer, nullptr);
                vkFreeMemory(device, memory, nullptr);
            }

            VkDeviceMemory memory = nullptr;
            VkBuffer buffer = nullptr;
            VkDevice device = nullptr;
        };

        BufferMemoryDeleter guard = { stagingBufferMemory, stagingBuffer, m_device };

        if (void* ptr; vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &ptr) == VK_SUCCESS)
        {
            memcpy(ptr, rawData.data(), (size_t)bufferSize);
            vkUnmapMemory(m_device, stagingBufferMemory);
        }
        else return {};

        if (bufferData.handle = vk::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | flag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferData.memory, m_device, m_GPU))
        {
            vk::copyBuffer(stagingBuffer, bufferData.handle, bufferSize, m_device, m_commandPool, m_queue);
            m_buffers.push_back(bufferData);

            return { bufferData.handle, bufferData.size };
        }

        return {};
    }

    void cleanup() noexcept;

private:
    VkPhysicalDevice m_GPU;
    VkDevice         m_device;
    VkQueue          m_queue;
    VkCommandPool    m_commandPool;

    struct BufferData
    {
        VkBuffer       handle = nullptr;
        VkDeviceMemory memory = nullptr;
        uint32_t       size   = 0;
    };

    std::vector<BufferData> m_buffers;
};

#endif // !VK_RESOURCE_HOLDER_HPP