#include "vulkan_api/wrappers/presentation/MainView.hpp"
#include "vulkan_api/wrappers/render/Render.hpp"


// TODO add clear color value
VkResult Render::begin(VkCommandBuffer cmd, const MainView& view, uint32_t imageIndex, VkImageView depthImageView) noexcept
{
    VkCommandBufferBeginInfo beginInfo = 
    {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = 0,
        .pInheritanceInfo = nullptr
    };

    if (auto result = vkBeginCommandBuffer(cmd, &beginInfo); result != VK_SUCCESS)
        return result;

    const VkImageMemoryBarrier imageMemoryBarrier =
    {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = VK_ACCESS_NONE,
        .dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = 0,
        .dstQueueFamilyIndex = 0,
        .image               = view.getImage(imageIndex),
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
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // srcStageMask
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
        0,
        0,
        nullptr,
        0,
        nullptr,
        1, // imageMemoryBarrierCount
        &imageMemoryBarrier // pImageMemoryBarriers
    );

    VkExtent2D extent = view.getExtent();

    const VkRenderingAttachmentInfoKHR colorAttachmentInfo = 
    {
        .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        .pNext              = VK_NULL_HANDLE,
        .imageView          = view.getImageView(imageIndex),
        .imageLayout        = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
        .resolveMode        = VK_RESOLVE_MODE_NONE,
        .resolveImageView   = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue         = {{ 0.0f, 0.0f, 0.0f, 1.0f }}
    };

    VkRenderingAttachmentInfoKHR depthAttachmentInfo = 
    {
        .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        .pNext              = VK_NULL_HANDLE,
        .imageView          = depthImageView,
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

    VkViewport viewport = 
    {
        .x = 0.f,
        .y = 0.f,
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = 
    {
        .offset = { 0, 0 },
        .extent = extent
    };

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    return VK_SUCCESS;
}


VkResult Render::end(VkCommandBuffer cmd, const MainView& view, uint32_t imageIndex) noexcept
{
    vkCmdEndRendering(cmd);

    const VkImageMemoryBarrier imageMemoryBarrier =
    {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask       = VK_ACCESS_NONE,
        .oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = 0,
        .dstQueueFamilyIndex = 0,
        .image               = view.getImage(imageIndex),
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
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // dstStageMask
        0,
        0,
        nullptr,
        0,
        nullptr,
        1, // imageMemoryBarrierCount
        &imageMemoryBarrier // pImageMemoryBarriers
    );

    return vkEndCommandBuffer(cmd);
}