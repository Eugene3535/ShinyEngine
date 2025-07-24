#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include <array>


class Swapchain
{
    static constexpr uint32_t buffer_count = 3;

public:

    Swapchain() noexcept;
    ~Swapchain();

    bool create(struct VkPhysicalDevice_T* phisycalDevice, struct VkDevice_T* logicalDevice, struct VkSurfaceKHR_T* surface) noexcept;
    void cleanup() noexcept;

    VkSwapchainKHR_T* handle;
    int32_t format;
    std::array<VkImage_T*, buffer_count> images;

private:
    struct VkDevice_T* m_logicalDevice;
};

#endif // !SWAPCHAIN_HPP