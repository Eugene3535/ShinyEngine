#ifndef MAIN_VIEW_HPP
#define MAIN_VIEW_HPP

#include <vector>

#include "vulkan_api/context/VulkanContext.hpp"


class MainView
{
public:
    MainView() noexcept;
    ~MainView();

    VkResult create(VulkanContext& context, struct GLFWwindow* window) noexcept;
    VkResult recreate(bool depth) noexcept;
    void     destroy()  noexcept;

    VkSwapchainKHR&   getSwapchain() noexcept;
    VkFormat          getFormat()    const noexcept;
    const VkExtent2D& getExtent()    const noexcept;

    VkImage     getImage(uint32_t index)     const noexcept;
    VkImageView getImageView(uint32_t index) const noexcept;
    VkImageView getDepthImageView()          const noexcept;

    VulkanContext* getContext() const noexcept;

private:
    void createDepthResources() noexcept;

    VulkanContext* m_context;

    VkSurfaceKHR   m_surface;
    VkSwapchainKHR m_swapchain;

    std::vector<VkImage>     m_images;
    std::vector<VkImageView> m_imageViews;

//  Depth buffer
    VkImage        m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkImageView    m_depthImageView;

    VkFormat   m_format;
    VkExtent2D m_extent;
};

#endif // !MAIN_VIEW_HPP