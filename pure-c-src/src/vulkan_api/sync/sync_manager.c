#include "vulkan_api/sync/sync_manager.h"


bool SyncManager_create(SyncManager* sync, VkDevice device)
{
    const VkSemaphoreCreateInfo semaphoreInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0
    };

    const VkFenceCreateInfo fenceInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if(vkCreateSemaphore(device, &semaphoreInfo, VK_NULL_HANDLE, &sync->imageAvailableSemaphores[i]) != VK_SUCCESS)
            return false;

        if(vkCreateSemaphore(device, &semaphoreInfo, VK_NULL_HANDLE, &sync->renderFinishedSemaphores[i]) != VK_SUCCESS)
            return false;

        if(vkCreateFence(device, &fenceInfo, VK_NULL_HANDLE, &sync->inFlightFences[i]) != VK_SUCCESS)
            return false;
    }

    return true;
}


void SyncManager_destroy(SyncManager* sync, VkDevice device)
{
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(device, sync->renderFinishedSemaphores[i], VK_NULL_HANDLE);
        vkDestroySemaphore(device, sync->imageAvailableSemaphores[i], VK_NULL_HANDLE);
        vkDestroyFence(device, sync->inFlightFences[i], VK_NULL_HANDLE);
    }
}