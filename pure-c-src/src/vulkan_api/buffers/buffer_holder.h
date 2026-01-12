#ifndef BUFFER_HOLDER_H
#define BUFFER_HOLDER_H

#include "vulkan_api/utils/tools.h"


typedef struct
{
    VkBuffer handle;
    uint32_t size;
} Buffer;


typedef struct
{
    VkBuffer       handle;
    VkDeviceMemory memory;
    uint32_t       size;
} BufferData;


typedef struct
{
    BufferData* data;
    uint32_t    size;
} BufferHolder;


Buffer BufferHolder_allocate(BufferHolder* holder, const void* data, uint32_t size, VkBufferUsageFlagBits flag, const void* ctx, VkCommandPool pool);
void BufferHolder_destroy(const BufferHolder* holder, VkDevice device);

#endif // !BUFFER_HOLDER_H