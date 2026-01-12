#ifndef MAIN_VIEW_H
#define MAIN_VIEW_H

#include <GLFW/glfw3.h>

#include "vulkan_api/context/context.h"


typedef struct
{
    VulkanContext* context;

    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;

    struct 
    {
        VkImage* data;
        uint32_t size;
    } images;

    struct 
    {
        VkImageView* data;
        uint32_t size;
    } imageViews;


//  Depth buffer
    VkImage        depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView    depthImageView;

    VkFormat   format;
    VkExtent2D extent;

} MainView;

bool MainView_createSurface(MainView* view, GLFWwindow* window);
bool MainView_recreate(MainView* view, bool useDepth);
void MainView_destroy(MainView* view);

#endif // !MAIN_VIEW_H