#pragma once

#include "vx_utils.hpp"

namespace VxEngine {

// A struct to hold an allocated image. Used for the engine draw image.
struct AllocatedImage{
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
    VkExtent3D extent;
    VkFormat format;
};

void copyImageToImage(VkCommandBuffer cmd, VkImage srcImage, VkImage dstImage, VkExtent2D srcExtent, VkExtent2D dstExtent);
void transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

}


