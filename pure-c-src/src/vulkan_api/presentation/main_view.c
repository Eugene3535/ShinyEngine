#include <limits.h>
#include <string.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <cglm/util.h>

#include "vulkan_api/utils/tools.h"
#include "vulkan_api/presentation/main_view.h"



typedef struct 
{
    VkSurfaceCapabilitiesKHR capabilities;

    struct
    {
        VkSurfaceFormatKHR* data;
        uint32_t size;
    } surfaceFormats;

    struct
    {
        VkPresentModeKHR* data;
        uint32_t size;
    } presentModes;

} SwapChainSupportDetails;


static VkPresentModeKHR SwapChainSupportDetails_getPresentMode(SwapChainSupportDetails* details)
{
    for (uint32_t i = 0; i < details->presentModes.size; ++i)
    {
        if(details->presentModes.data[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return details->presentModes.data[i];
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}


static VkSurfaceFormatKHR SwapChainSupportDetails_getSurfaceFormat(SwapChainSupportDetails* details)
{
    for (uint32_t i = 0; i < details->surfaceFormats.size; ++i)
    {
        if(details->surfaceFormats.data[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            details->surfaceFormats.data[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return details->surfaceFormats.data[i];
    }

    return details->surfaceFormats.data[0];
}


static SwapChainSupportDetails* query_swapchain_support(MainView* view)
{
    VkPhysicalDevice gpu = view->context->GPU;
    VkSurfaceKHR surface = view->surface;

    SwapChainSupportDetails* details = calloc(1, sizeof(SwapChainSupportDetails));
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details->capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, VK_NULL_HANDLE);

    if (formatCount) 
    {
        details->surfaceFormats.data = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
        details->surfaceFormats.size = formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, details->surfaceFormats.data);
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, VK_NULL_HANDLE);

    if (presentModeCount) 
    {
        details->presentModes.data = malloc(presentModeCount * sizeof(VkPresentModeKHR));
        details->presentModes.size = presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, details->presentModes.data);
    }

    return details;
}


static VkExtent2D choose_swap_extent(const SwapChainSupportDetails* details, const VkExtent2D* currentExtent)
{
    VkExtent2D actualExtent = { 0, 0 };

    if (details->capabilities.currentExtent.width != UINT_MAX)
    {
        return details->capabilities.currentExtent;
    }
    else
    {
        memcpy(&actualExtent, currentExtent, sizeof(VkExtent2D));
        actualExtent.width  = glm_clamp(actualExtent.width, details->capabilities.minImageExtent.width, details->capabilities.maxImageExtent.width);
        actualExtent.height = glm_clamp(actualExtent.height, details->capabilities.minImageExtent.height, details->capabilities.maxImageExtent.height);
    }

    return actualExtent;
}


static bool create_depth_resources(MainView* view)
{
    bool result = false;
    VkDevice device = view->context->device;
    VkFormat depthFormat = find_depth_format(view->context->GPU);

    if(depthFormat != VK_FORMAT_UNDEFINED)
    {
        result = create_image_2D(
            view->extent, 
            depthFormat, 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            &view->depthImage, 
            &view->depthImageMemory, 
            view->context->GPU, 
            device);

        if(result)
            return create_image_view_2D(device, view->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &view->depthImageView);
    }

    return result;
}



bool MainView_createSurface(MainView* view, GLFWwindow* window)
{
    bool result = false;

#ifdef _WIN32
    const VkWin32SurfaceCreateInfoKHR surfaceInfo = 
    {
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext     = VK_NULL_HANDLE,
        .flags     = 0,
        .hinstance = GetModuleHandle(VK_NULL_HANDLE),
        .hwnd      = glfwGetWin32Window(window)
    };

    if(vkCreateWin32SurfaceKHR(view->context->instance, &surfaceInfo, VK_NULL_HANDLE, &view->surface) == VK_SUCCESS)
        result = MainView_recreate(view, true);
#endif

#ifdef __linux__
    xcb_connection_t* connection = xcb_connect(VK_NULL_HANDLE, VK_NULL_HANDLE);

    if (xcb_connection_has_error(connection))
        return false;

    const VkXcbSurfaceCreateInfoKHR surfaceInfo =
    {
        .sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .pNext      = VK_NULL_HANDLE,
        .flags      = 0,
        .connection = connection,
        .window     = (xcb_window_t)glfwGetX11Window(window)
    };

    if (vkCreateXcbSurfaceKHR(view->context->instance, &surfaceInfo, VK_NULL_HANDLE, &view->surface) == VK_SUCCESS)
        result = MainView_recreate(view, true);
#endif


    return result;
}


bool MainView_recreate(MainView* view, bool useDepth)
{
    bool result = false;
    SwapChainSupportDetails* swapChainSupport = VK_NULL_HANDLE;

    if(view->context && view->surface)
    {
        VkDevice device = view->context->device;

        if(view->swapchain)
        {
            for(uint32_t i = 0; i < view->images.size; ++i)
                vkDestroyImageView(device, view->imageViews.data[i], VK_NULL_HANDLE);

            vkDestroySwapchainKHR(device, view->swapchain, VK_NULL_HANDLE);
            view->swapchain = VK_NULL_HANDLE;
        }

        swapChainSupport = query_swapchain_support(view);
        const uint32_t minImageCount = swapChainSupport->capabilities.minImageCount;

        view->format = SwapChainSupportDetails_getSurfaceFormat(swapChainSupport).format;
        view->extent = choose_swap_extent(swapChainSupport, &view->extent);

        const VkSwapchainCreateInfoKHR swapchainInfo = 
        {
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext                 = VK_NULL_HANDLE,
            .flags                 = 0,
            .surface               = view->surface,
            .minImageCount         = minImageCount,
            .imageFormat           = view->format,
            .imageColorSpace       = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            .imageExtent           = view->extent,
            .imageArrayLayers      = 1,
            .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = VK_NULL_HANDLE,
            .preTransform          = swapChainSupport->capabilities.currentTransform,
            .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode           = SwapChainSupportDetails_getPresentMode(swapChainSupport),
            .clipped               = VK_TRUE,
            .oldSwapchain          = view->swapchain
        };

        if (vkCreateSwapchainKHR(device, &swapchainInfo, VK_NULL_HANDLE, &view->swapchain) == VK_SUCCESS)
        {
            uint32_t imageCount = 0;
            
            if(vkGetSwapchainImagesKHR(device, view->swapchain, &imageCount, VK_NULL_HANDLE) != VK_SUCCESS)
                return false;

            if(view->images.size == 0)
            {
                view->images.data = malloc(imageCount * sizeof(VkImage));
                view->images.size = imageCount;
            }

            if(view->imageViews.size == 0)
            {
                view->imageViews.data = malloc(imageCount * sizeof(VkImageView));
                view->imageViews.size = imageCount;
            }

            if (vkGetSwapchainImagesKHR(device, view->swapchain, &imageCount, view->images.data) == VK_SUCCESS)
            {
                for (uint32_t i = 0; i < view->imageViews.size; ++i)
                {
                    if (!create_image_view_2D(device, view->images.data[i], view->format, VK_IMAGE_ASPECT_COLOR_BIT, &view->imageViews.data[i]))
                        result = false;  
                }

                if (useDepth)
                {
                    if (view->depthImageView)
                        vkDestroyImageView(device, view->depthImageView, VK_NULL_HANDLE);

                    if (view->depthImage)
                        vkDestroyImage(device, view->depthImage, VK_NULL_HANDLE);

                    if (view->depthImageMemory)
                        vkFreeMemory(device, view->depthImageMemory, VK_NULL_HANDLE);

                    if(!create_depth_resources(view))
                        return false;
                }

                result = true;
            }
        }
    }

    if(swapChainSupport)
    {
        free(swapChainSupport->surfaceFormats.data);
        free(swapChainSupport->presentModes.data);
        free(swapChainSupport);
    }

    return result;
}


void MainView_destroy(MainView* view)
{
    VkDevice device = view->context->device;

    if(view->swapchain)
    {
        for (uint32_t i = 0; i < view->imageViews.size; ++i)
            vkDestroyImageView(device, view->imageViews.data[i], VK_NULL_HANDLE);

        vkDestroySwapchainKHR(device, view->swapchain, VK_NULL_HANDLE);
    }

    if (view->depthImageView)
        vkDestroyImageView(device, view->depthImageView, VK_NULL_HANDLE);

    if (view->depthImage)
        vkDestroyImage(device, view->depthImage, VK_NULL_HANDLE);

    if (view->depthImageMemory)
        vkFreeMemory(device, view->depthImageMemory, VK_NULL_HANDLE);

    if(view->surface)
        vkDestroySurfaceKHR(view->context->instance, view->surface, VK_NULL_HANDLE);
    
    free(view->images.data);
    free(view->imageViews.data);
}