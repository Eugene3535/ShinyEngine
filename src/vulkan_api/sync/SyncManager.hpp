#ifndef SYNC_MANAGER_HPP
#define SYNC_MANAGER_HPP

#include <array>

#include "vulkan_api/utils/Defines.hpp"


class SyncManager
{
public:
    SyncManager() noexcept;
    ~SyncManager();

    bool create(struct VkDevice_T* logicalDevice) noexcept;
    void destroy(struct VkDevice_T* logicalDevice) noexcept;

    std::array<struct VkSemaphore_T*, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
    std::array<struct VkSemaphore_T*, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
    std::array<struct VkFence_T*, MAX_FRAMES_IN_FLIGHT>     inFlightFences;
    uint32_t                                                currentFrame;
};

#endif // !SYNC_MANAGER_HPP