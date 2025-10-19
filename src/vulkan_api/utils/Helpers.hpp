#ifndef VULKAN_HELPERS_HPP
#define VULKAN_HELPERS_HPP

#include <cstdint>
#include <span>

#include <vulkan/vulkan.h>

#include "vulkan_api/utils/Defines.hpp"

BEGIN_NAMESPACE_VK


uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice GPU) noexcept;


VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool pool) noexcept;
void endSingleTimeCommands(VkCommandBuffer cmd, VkDevice device, VkCommandPool pool, VkQueue queue) noexcept;


VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& bufferMemory, VkDevice device, VkPhysicalDevice GPU) noexcept;
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDevice device, VkCommandPool pool, VkQueue queue) noexcept;


bool transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkDevice device, VkCommandPool pool, VkQueue queue) noexcept;
bool copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkDevice device, VkCommandPool pool, VkQueue queue) noexcept;
VkResult createImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkPhysicalDevice GPU, VkDevice device) noexcept;
VkResult createImageView2D(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView) noexcept;


VkFormat findSupportedFormat(std::span<const VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice GPU) noexcept;
VkFormat findDepthFormat(VkPhysicalDevice GPU) noexcept;
bool hasStencilComponent(VkFormat format) noexcept;


END_NAMESPACE_VK

#endif // !VULKAN_HELPERS_HPP