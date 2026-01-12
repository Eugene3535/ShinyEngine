#include "vulkan_api/presentation/main_view.h"
#include "vulkan_api/render/render.h"


// TODO add clear color value
bool render_begin(VkCommandBuffer cmd, const MainView* view, uint32_t imageIndex)
{
    const VkCommandBufferBeginInfo beginInfo = 
    {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = VK_NULL_HANDLE,
        .flags            = 0,
        .pInheritanceInfo = VK_NULL_HANDLE
    };

    VkResult result = vkBeginCommandBuffer(cmd, &beginInfo);

    if (result != VK_SUCCESS)
        return false;

    const VkImageMemoryBarrier imageMemoryBarrier =
    {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = VK_NULL_HANDLE,
        .srcAccessMask       = VK_ACCESS_NONE,
        .dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = 0,
        .dstQueueFamilyIndex = 0,
        .image               = view->images.data[imageIndex],
        .subresourceRange =     
        {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };

    vkCmdPipelineBarrier(
                            cmd,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,             // srcStageMask
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
                            0,
                            0,
                            VK_NULL_HANDLE,
                            0,
                            VK_NULL_HANDLE,
                            1,                                             // imageMemoryBarrierCount
                            &imageMemoryBarrier                            // pImageMemoryBarriers
    );

    VkExtent2D extent = view->extent;

    const VkRenderingAttachmentInfoKHR colorAttachmentInfo = 
    {
        .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        .pNext              = VK_NULL_HANDLE,
        .imageView          = view->imageViews.data[imageIndex],
        .imageLayout        = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
        .resolveMode        = VK_RESOLVE_MODE_NONE,
        .resolveImageView   = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue         = {{ 0.f, 0.f, 0.f, 1.f }}
    };

    const VkRenderingAttachmentInfoKHR depthAttachmentInfo = 
    {
        .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        .pNext              = VK_NULL_HANDLE,
        .imageView          = view->depthImageView,
        .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .resolveMode        = VK_RESOLVE_MODE_NONE,
        .resolveImageView   = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp            = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue         = { 1.f, 0.f }
    };

    const VkRenderingInfoKHR renderingInfo =
    {
        .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        .pNext                = VK_NULL_HANDLE,
        .flags                = 0,
        .renderArea           = { { 0, 0 }, extent },
        .layerCount           = 1,
        .viewMask             = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &colorAttachmentInfo,
        .pDepthAttachment     = &depthAttachmentInfo,
        .pStencilAttachment   = VK_NULL_HANDLE
    };

    vkCmdBeginRendering(cmd, &renderingInfo);

    const VkViewport viewport = 
    {
        .x        = 0.f,
        .y        = 0.f,
        .width    = (float)extent.width,
        .height   = (float)extent.height,
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    const VkRect2D scissor = 
    {
        .offset = { 0, 0 },
        .extent = extent
    };

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    return true;
}


bool render_end(VkCommandBuffer cmd, const MainView* view, uint32_t imageIndex)
{
    vkCmdEndRendering(cmd);

    const VkImageMemoryBarrier imageMemoryBarrier =
    {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = VK_NULL_HANDLE,
        .srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask       = VK_ACCESS_NONE,
        .oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = 0,
        .dstQueueFamilyIndex = 0,
        .image               = view->images.data[imageIndex],
        .subresourceRange =     
        {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };

    vkCmdPipelineBarrier(
                            cmd,
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,   // srcStageMask
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,            // dstStageMask
                            0,
                            0,
                            VK_NULL_HANDLE,
                            0,
                            VK_NULL_HANDLE,
                            1,                                               // imageMemoryBarrierCount
                            &imageMemoryBarrier                              // pImageMemoryBarriers
    );

    return (vkEndCommandBuffer(cmd) == VK_SUCCESS);
}