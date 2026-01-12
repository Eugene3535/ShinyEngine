#include "vulkan_api/context/context.h"
#include "vulkan_api/buffers/buffer_holder.h"


Buffer BufferHolder_allocate(BufferHolder* holder, const void* data, uint32_t size, VkBufferUsageFlagBits flag, const void* ctx, VkCommandPool pool)
{
    const VulkanContext* context = (const VulkanContext*)ctx;
    Buffer result = { VK_NULL_HANDLE, 0 };

    BufferData bufferData = { VK_NULL_HANDLE, VK_NULL_HANDLE, size };
    VkDeviceSize bufferSize = 0;

    if(flag == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
        bufferSize = sizeof(float) * size;

    if(flag == VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
        bufferSize = sizeof(uint32_t) * size;

    VkDeviceMemory stagingBufferMemory;
    VkBuffer stagingBuffer = create_buffer(
                                            bufferSize, 
                                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                                            &stagingBufferMemory, 
                                            context->device, 
                                            context->GPU);

    if(!stagingBuffer)
        return result;

    {
        void* ptr;

        if(vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &ptr) == VK_SUCCESS)
        {
            memcpy(ptr, data, bufferSize);
            vkUnmapMemory(context->device, stagingBufferMemory);
        }
        else goto error_create_buffer;
    }

    bufferData.handle = create_buffer(
                                        bufferSize, 
                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | flag, 
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                                        &bufferData.memory, 
                                        context->device, 
                                        context->GPU);

    if(bufferData.handle)
    {
        copy_buffer(stagingBuffer, bufferData.handle, bufferSize, context->device, pool, context->queue);

        BufferData* reallocData = realloc(holder->data, (holder->size + 1) * sizeof(BufferData));

        if(reallocData)
        {
            memcpy(reallocData + holder->size, &bufferData, sizeof(BufferData));
            holder->data = reallocData;
            holder->size += 1;

            result.handle = bufferData.handle;
            result.size = size;
        }
    }

error_create_buffer:
    vkDestroyBuffer(context->device, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(context->device, stagingBufferMemory, VK_NULL_HANDLE);

    return result;
}


void BufferHolder_destroy(const BufferHolder* holder, VkDevice device)
{
    for (uint32_t i = 0; i < holder->size; ++i)
    {
        vkDestroyBuffer(device, holder->data[i].handle, VK_NULL_HANDLE);
        vkFreeMemory(device, holder->data[i].memory, VK_NULL_HANDLE);
    }

    free(holder->data);
}