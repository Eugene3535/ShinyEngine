#ifndef SYNC_MANAGER_HPP
#define SYNC_MANAGER_HPP

#include <stdbool.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include "vulkan_api/utils/defines.h"


typedef struct
{
    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence     inFlightFences[MAX_FRAMES_IN_FLIGHT];
    uint32_t    currentFrame;
} SyncManager;


bool SyncManager_create(SyncManager* sync, VkDevice device);
void SyncManager_destroy(SyncManager* sync, VkDevice device);

#endif // !SYNC_MANAGER_HPP