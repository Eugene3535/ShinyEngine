#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include <vulkan/vulkan.h>


typedef struct
{
    VkDeviceMemory imageMemory;
    VkImage        image;
    VkImageView    imageView;
    VkSampler      sampler;
} Texture2D;


bool Texture2D_loadFromFile(Texture2D* texture, const char* filepath, const void* ctx, VkCommandPool pool);
void Texture2D_destroy(Texture2D* texture, VkDevice device);


#endif // !TEXTURE2D_H