#ifndef STRUCTURS_HPP
#define STRUCTURS_HPP

#include <vector>

#include <vulkan/vulkan.h>


struct SwapChainSupportDetails 
{
    VkPresentModeKHR   getPresentMode()   const noexcept;
    VkSurfaceFormatKHR getSurfaceFormat() const noexcept;

    VkSurfaceCapabilitiesKHR		capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>	presentModes;
};

#endif // !STRUCTURS_HPP