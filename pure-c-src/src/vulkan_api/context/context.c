#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#include "vulkan_api/context/context.h"


#ifdef DEBUG
    static const char* validation_layers[] = 
    {
        "VK_LAYER_KHRONOS_validation"
    };

    static const uint32_t validation_layer_count = sizeof(validation_layers) / sizeof(const char*);


    static VkResult check_validation_layer_support()
    {
        VkResult all_layers_found = VK_SUCCESS;

        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, VK_NULL_HANDLE);

        VkLayerProperties* availableLayers = (VkLayerProperties*) malloc(sizeof(VkLayerProperties) * layerCount);

        if(!availableLayers)
            return false;

        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

        for(uint32_t i = 0; i < validation_layer_count; ++i)
        {
            const char* layerName = validation_layers[i];
            bool layerFound = false;

            for(uint32_t j = 0; j < layerCount; ++j)
            {
                if (strcmp(layerName, availableLayers[j].layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if(!layerFound)
                all_layers_found = VK_ERROR_LAYER_NOT_PRESENT;
        }

        free(availableLayers);

        return all_layers_found;
    }


    static uint32_t debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageType, 
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
        void* pUserData)
    {
        printf("validation layer: %s\n", pCallbackData->pMessage);

        return VK_FALSE;
    }
#endif // !DEBUG


static bool create_instance(VulkanContext* context);
static bool select_videocard(VulkanContext* context);
static bool create_device(VulkanContext* context);


bool VulkanContext_create(VulkanContext* context)
{
    if(create_instance(context))
        if(select_videocard(context))
            if(create_device(context))
                return true;

    return false;
}


void VulkanContext_destroy(VulkanContext* context)
{
    if(context->device)
        vkDestroyDevice(context->device, VK_NULL_HANDLE);

    if(context->instance)
        vkDestroyInstance(context->instance, VK_NULL_HANDLE);
}


bool create_instance(VulkanContext* context)
{
    VkResult result = VK_SUCCESS;

#ifdef DEBUG
    if (check_validation_layer_support() != result)
        return false;
#endif // !DEBUG

    const char* requiredExtensions[3] = 
    {
        VK_KHR_SURFACE_EXTENSION_NAME, NULL, NULL
    };

    uint32_t requiredExtensionCount = 1;

#ifdef _WIN32
    requiredExtensions[requiredExtensionCount] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    ++requiredExtensionCount;
#endif

#ifdef __linux__
    requiredExtensions[requiredExtensionCount] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
    ++requiredExtensionCount;
#endif

#ifdef DEBUG
    requiredExtensions[requiredExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    ++requiredExtensionCount;
#endif

    uint32_t availableExtensionCount;
    vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &availableExtensionCount, VK_NULL_HANDLE);

    VkExtensionProperties* availableExtensions = malloc(availableExtensionCount * sizeof(VkExtensionProperties));

    if(!availableExtensions)
        return false;

    vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &availableExtensionCount, availableExtensions);

    const char** deviceExtensions = (const char**) malloc(availableExtensionCount * sizeof(const char*));

    if(!deviceExtensions)
    {
        result = VK_ERROR_INITIALIZATION_FAILED;
        goto error_malloc;
    }

    for(uint32_t i = 0; i < availableExtensionCount; ++i)
        deviceExtensions[i] = availableExtensions[i].extensionName;

    for (uint32_t i = 0; i < requiredExtensionCount; ++i)
    {
        const char* extension = requiredExtensions[i];
        bool extensionFound = false;

        for (uint32_t j = 0; j < availableExtensionCount; ++j)
        {
            if (strcmp(extension, deviceExtensions[j]) == 0)
            {
                extensionFound = true;
                break;
            }
        }

        if(!extensionFound)
            result = VK_ERROR_INITIALIZATION_FAILED;
    }

error_malloc:
    free(availableExtensions);
    free(deviceExtensions);

    if(result != VK_SUCCESS)
        return false;

    const VkApplicationInfo appInfo = 
    {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = VK_NULL_HANDLE,
        .pApplicationName   = "Star Dust",
        .applicationVersion = 1,
        .pEngineName        = "Shining Engine",
        .engineVersion      = 1,
        .apiVersion         = VK_API_VERSION_1_3
    };

    VkInstanceCreateInfo instanceInfo = 
    {
        .sType =                   VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext =                   VK_NULL_HANDLE,
        .flags =                   0,
        .pApplicationInfo =        &appInfo,
        .enabledLayerCount =       0,
        .ppEnabledLayerNames =     VK_NULL_HANDLE,
        .enabledExtensionCount =   requiredExtensionCount,
        .ppEnabledExtensionNames = requiredExtensions
    };

#ifdef DEBUG
    instanceInfo.enabledLayerCount = validation_layer_count;
    instanceInfo.ppEnabledLayerNames = validation_layers;

    const VkDebugUtilsMessengerCreateInfoEXT debugInfo = 
    {
       .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
       .pNext           = VK_NULL_HANDLE,
       .flags           = 0,
       .messageSeverity = 0,
       .messageType     = 0,
       .pfnUserCallback = debug_callback,
       .pUserData = VK_NULL_HANDLE
    };

    instanceInfo.pNext = (const void*)(&debugInfo);
#endif // !DEBUG

    result = vkCreateInstance(&instanceInfo, VK_NULL_HANDLE, &context->instance);

    return (result == VK_SUCCESS);
}


bool select_videocard(VulkanContext* context)
{    
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(context->instance, &deviceCount, VK_NULL_HANDLE);

    if (deviceCount)
    {
        VkPhysicalDevice* devices = malloc(deviceCount * sizeof(VkPhysicalDevice));

        if(devices)
        {
            vkEnumeratePhysicalDevices(context->instance, &deviceCount, devices);

            for(size_t i = 0; i < deviceCount; ++i)
            {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(devices[i], &properties);

                if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                    context->GPU = devices[i];

                if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    context->GPU = devices[i];
                    break;
                }
            }

            free(devices);
        }
    }

    return context->GPU ? true : false;
}


bool create_device(VulkanContext* context)
{
    VkPhysicalDeviceFeatures supportedFeatures;
    VkPhysicalDeviceFeatures enabledFeatures = {0};

    vkGetPhysicalDeviceFeatures(context->GPU, &supportedFeatures);

    if (supportedFeatures.samplerAnisotropy)
        enabledFeatures.samplerAnisotropy = VK_TRUE;

    if (supportedFeatures.fillModeNonSolid)
        enabledFeatures.fillModeNonSolid = VK_TRUE;

    {// Find main queue family index
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(context->GPU, &queueFamilyCount, VK_NULL_HANDLE);

        VkQueueFamilyProperties* queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(context->GPU, &queueFamilyCount, queueFamilies);

        context->mainQueueFamilyIndex = UINT32_MAX;

        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            if (queueFamilies[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT))
            {
                context->mainQueueFamilyIndex = i;
                break;
            }
        }

        free(queueFamilies);
    }

	if(context->mainQueueFamilyIndex != UINT32_MAX)
    {
        const float queuePriority = 1.0f;

    	const VkDeviceQueueCreateInfo queueInfo = 
        {
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext            = VK_NULL_HANDLE,
            .flags            = 0,
            .queueFamilyIndex = context->mainQueueFamilyIndex,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority
        };

        const char* requiredExtensions[] = 
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };
        uint32_t requiredExtensionCount = sizeof(requiredExtensions) / sizeof(const char*);

        uint32_t availableExtensionCount;
        vkEnumerateDeviceExtensionProperties(context->GPU, VK_NULL_HANDLE, &availableExtensionCount, VK_NULL_HANDLE);

        VkExtensionProperties* availableExtensions = malloc(availableExtensionCount * sizeof(VkExtensionProperties));
        vkEnumerateDeviceExtensionProperties(context->GPU, VK_NULL_HANDLE, &availableExtensionCount, availableExtensions);

        VkResult result = VK_SUCCESS;

        for (uint32_t i = 0; i < requiredExtensionCount; ++i)
        {
            bool extensionFound = false;

            for (uint32_t j = 0; j < availableExtensionCount; ++j)
            {
                if (strcmp(requiredExtensions[i], availableExtensions[j].extensionName) == 0)
                {
                    extensionFound = true;
                    break;
                }
            }

            if(!extensionFound)
                result = VK_ERROR_INITIALIZATION_FAILED;
        }

        free(availableExtensions);

        if(result != VK_SUCCESS)
            return false;

        const VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature = 
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .dynamicRendering = VK_TRUE
        };

        VkDeviceCreateInfo deviceInfo = 
        {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext                   = &dynamicRenderingFeature,
            .flags                   = 0,
            .queueCreateInfoCount    = 1,
            .pQueueCreateInfos       = &queueInfo,
            .enabledLayerCount       = 0,
            .ppEnabledLayerNames     = VK_NULL_HANDLE,
            .enabledExtensionCount   = requiredExtensionCount,
            .ppEnabledExtensionNames = requiredExtensions,
            .pEnabledFeatures        = &enabledFeatures
        };
#ifdef DEBUG
    	deviceInfo.enabledLayerCount   = validation_layer_count;
    	deviceInfo.ppEnabledLayerNames = validation_layers;
#endif

        if(vkCreateDevice(context->GPU, &deviceInfo, VK_NULL_HANDLE, &context->device) == VK_SUCCESS)
        {
            vkGetDeviceQueue(context->device, context->mainQueueFamilyIndex, 0, &context->queue);

            return true;
        }
    }

    return false;
}