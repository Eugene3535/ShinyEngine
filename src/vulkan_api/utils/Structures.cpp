#include "vulkan_api/utils/Structures.hpp"


VkPresentModeKHR SwapChainSupportDetails::getPresentMode() const noexcept
{
	for (const auto& availablePresentMode : presentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}


VkSurfaceFormatKHR SwapChainSupportDetails::getSurfaceFormat() const noexcept
{
	for (const auto& availableFormat : formats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return formats[0];
}