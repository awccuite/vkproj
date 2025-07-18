#include "vx_image.hpp"

namespace VxEngine {
    
    // Blit image to image. Slower than cmdCopyImage2, but blit is more flexible so easier to get started with.
    void copyImageToImage(VkCommandBuffer cmd, VkImage srcImage, VkImage dstImage, VkExtent2D srcExtent, VkExtent2D dstExtent){
        VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

        blitRegion.srcOffsets[1].x = srcExtent.width;
        blitRegion.srcOffsets[1].y = srcExtent.height;
        blitRegion.srcOffsets[1].z = 1;

        blitRegion.dstOffsets[1].x = dstExtent.width;
        blitRegion.dstOffsets[1].y = dstExtent.height;
        blitRegion.dstOffsets[1].z = 1;

        VkImageSubresourceLayers srcSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 };
        VkImageSubresourceLayers dstSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 };

        blitRegion.srcSubresource = srcSubresource;
        blitRegion.dstSubresource = dstSubresource;

        VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
        blitInfo.dstImage = dstImage;
        blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        blitInfo.srcImage = srcImage;
        blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        blitInfo.filter = VK_FILTER_LINEAR;
        blitInfo.regionCount = 1;
        blitInfo.pRegions = &blitRegion;

        vkCmdBlitImage2(cmd, &blitInfo);
    }

    // Transition image layout from one layout to another.
    // This is a temporary, simple, stupid implementation with performance implications.
    // TODO: Implement a more efficient way to transition image layouts.
    void transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout){
        VkImageMemoryBarrier2 imageBarrier {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        imageBarrier.pNext = nullptr;
    
        imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
    
        imageBarrier.oldLayout = currentLayout;
        imageBarrier.newLayout = newLayout;
    
        VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    
        // VkImageSubresourceRange subImage{};
        // subImage.aspectMask = aspectMask;
        // subImage.baseMipLevel = 0;
        // subImage.levelCount = VK_REMAINING_MIP_LEVELS;
        // subImage.baseArrayLayer = 0;
        // subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;
        VkImageSubresourceRange subImage = createImageSubresourceRange(aspectMask);
    
        imageBarrier.subresourceRange = subImage;
        imageBarrier.image = image;
    
        VkDependencyInfo depInfo {};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.pNext = nullptr;
    
        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &imageBarrier;
    
        vkCmdPipelineBarrier2(cmd, &depInfo);
    }

}
