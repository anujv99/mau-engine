#include "vulkan-image.h"

#include <engine/assert.h>

#include "vulkan-state.h"

namespace mau {
  
  Image::Image(TUint32 width, TUint32 height, TUint32 depth, TUint32 mip_levels, TUint32 array_layers, VkImageType type, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage):
    m_Format(format), m_SampleCount(samples) {
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage                   = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags                   = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    VkImageCreateInfo create_info      = {};
    create_info.sType                  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.pNext                  = nullptr;
    create_info.flags                  = 0u;
    create_info.imageType              = type;
    create_info.format                 = format;
    create_info.extent                 = { width, height, depth };
    create_info.mipLevels              = mip_levels;
    create_info.arrayLayers            = array_layers;
    create_info.samples                = samples;
    create_info.tiling                 = tiling;
    create_info.usage                  = usage;
    create_info.sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount  = 0u;
    create_info.pQueueFamilyIndices    = nullptr;
    create_info.initialLayout          = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CALL(vmaCreateImage(VulkanState::Ref().GetVulkanMemoryAllocator(), &create_info, &alloc_info, &m_Image, &m_Allocation, nullptr));
  }

  Image::Image(VkImage image, VkFormat format, VkSampleCountFlagBits samples): m_Image(image), m_Format(format), m_SampleCount(samples) {
    ASSERT(m_Image != VK_NULL_HANDLE);
  }

  Image::~Image() {
    if (m_Allocation) vmaDestroyImage(VulkanState::Ref().GetVulkanMemoryAllocator(), m_Image, m_Allocation);
  }

  ImageView::ImageView(VkImage image, VkFormat format, VkImageViewType view_type, VkImageAspectFlags aspect_mask) {
    CreateImageView(image, format, view_type, aspect_mask);
  }

  ImageView::ImageView(Handle<Image> image, VkImageViewType view_type, VkImageAspectFlags aspect_mask) {
    CreateImageView(image->GetImage(), image->GetFormat(), view_type, aspect_mask);
  }

  ImageView::ImageView(const ImageView& other) {
    m_ImageView = other.m_ImageView;
  }

  ImageView::~ImageView() {
    vkDestroyImageView(VulkanState::Ref().GetDevice(), m_ImageView, nullptr);
  }

  void ImageView::CreateImageView(VkImage image, VkFormat format, VkImageViewType view_type, VkImageAspectFlags aspect_mask) {
    ASSERT(image != VK_NULL_HANDLE);

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
    create_info.subresourceRange.aspectMask     = aspect_mask;
    create_info.subresourceRange.baseMipLevel   = 0u;
    create_info.subresourceRange.levelCount     = 1u;
    create_info.subresourceRange.baseArrayLayer = 0u;
    create_info.subresourceRange.layerCount     = 1u;

    VK_CALL(vkCreateImageView(VulkanState::Ref().GetDevice(), &create_info, nullptr, &m_ImageView));
  }

  Framebuffer::Framebuffer(std::vector<Handle<ImageView>> image_views, Handle<Renderpass> renderpass, TUint32 width, TUint32 height) {
    std::vector<VkImageView> image_views_handle(image_views.size());
    for (size_t i = 0; i < image_views.size(); i++) {
      image_views_handle[i] = image_views[i]->GetImageView();
    }
    CreateFramebuffer(image_views_handle, renderpass->Get(), width, height);
  }

  Framebuffer::Framebuffer(Handle<ImageView> color, Handle<ImageView> depth, Handle<Renderpass> renderpass, TUint32 width, TUint32 height) {
    std::vector<VkImageView> image_views_handle = { color->GetImageView(), depth->GetImageView()};
    CreateFramebuffer(image_views_handle, renderpass->Get(), width, height);
  }

  Framebuffer::~Framebuffer() {
    vkDestroyFramebuffer(VulkanState::Ref().GetDevice(), m_Framebuffer, nullptr);
  }

  void Framebuffer::CreateFramebuffer(std::vector<VkImageView> image_views, VkRenderPass renderpass, TUint32 width, TUint32 height) {
    ASSERT(renderpass != nullptr);
    
    VkFramebufferCreateInfo create_info = {};
    create_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.pNext                   = nullptr;
    create_info.flags                   = 0u;
    create_info.renderPass              = renderpass;
    create_info.attachmentCount         = static_cast<uint32_t>(image_views.size());
    create_info.pAttachments            = image_views.data();
    create_info.width                   = width;
    create_info.height                  = height;
    create_info.layers                  = 1u;

    VK_CALL(vkCreateFramebuffer(VulkanState::Ref().GetDevice(), &create_info, nullptr, &m_Framebuffer));
  }

}
