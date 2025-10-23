#ifndef RENDER_HPP
#define RENDER_HPP

#include <vulkan/vulkan.h>


class Render
{
public:
    static VkResult begin(VkCommandBuffer cmd, const class MainView& view, uint32_t imageIndex) noexcept;
    static VkResult end(VkCommandBuffer cmd, const class MainView& view, uint32_t imageIndex) noexcept;
};

#endif // !RENDER_HPP