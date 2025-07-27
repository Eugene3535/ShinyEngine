#ifndef LOGICAL_DEVICE_HPP
#define LOGICAL_DEVICE_HPP

#include <cstdint>

class LogicalDevice
{
public:
    LogicalDevice() noexcept;
    ~LogicalDevice();

    bool create(struct VkPhysicalDevice_T* physicalDevice) noexcept;
    void destroy() noexcept;

    struct VkDevice_T* handle;
    struct VkQueue_T*  queue;
    uint32_t mainQueueFamilyIndex;
};

#endif // !LOGICAL_DEVICE_HPP