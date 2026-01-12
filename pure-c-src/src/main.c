#ifdef _WIN32
#include <windows.h>
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif

#include "app/application.h"

int main()
{
    const char title[] = "Star Dust";
    const int width = 800;
    const int height = 600;

    VulkanApp app = {0};

    if(!VulkanApp_create(title, width, height, &app))
        return 1;

    int retCode = VulkanApp_run(&app);

    VulkanApp_destroy(&app);

    return retCode;
}