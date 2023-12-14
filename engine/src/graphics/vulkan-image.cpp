#include "vulkan-image.h"

#include <engine/assert.h>

namespace mau {

  ImageView::ImageView(VkImage image, VkFormat format, VkImageViewType view_type, VkDevice device): m_Device(device) {
    ASSERT(image != VK_NULL_HANDLE);
    ASSERT(m_Device != VK_NULL_HANDLE);

    VkImageViewCreateInfo create_info           = {};
    create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.pNext                           = nullptr;
    create_info.flags                           = 0u;
    create_info.image                           = image;
    create_info.viewType                        = view_type;
    create_info.format                          = format;
    create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel   = 0u;
    create_info.subresourceRange.levelCount     = 1u;
    create_info.subresourceRange.baseArrayLayer = 0u;
    create_info.subresourceRange.layerCount     = 1u;

    VK_CALL(vkCreateImageView(m_Device, &create_info, nullptr, &m_ImageView));
  }

  ImageView::ImageView(const ImageView& other) {
    m_Device = other.m_Device;
    m_ImageView = other.m_ImageView;
  }

  ImageView::~ImageView() {
    vkDestroyImageView(m_Device, m_ImageView, nullptr);
  }

}
