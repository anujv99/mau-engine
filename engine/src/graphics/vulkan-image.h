#pragma once

#include <vector>
#include <engine/types.h>
#include "common.h"
#include "vulkan-renderpass.h"

namespace mau {

  // image
  class Image: public HandledObject {
  public:
    Image(TUint32 width, TUint32 height, TUint32 depth, TUint32 mip_levels, TUint32 array_layers, VkImageType type, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
    Image(VkImage image, VkFormat format, VkSampleCountFlagBits samples);
    ~Image();
  public:
    inline VkImage GetImage() const { return m_Image; }
    inline VkFormat GetFormat() const { return m_Format; }
    inline VkSampleCountFlagBits GetSamples() const { return m_SampleCount; }
  private:
    VkImage               m_Image       = VK_NULL_HANDLE;
    VmaAllocation         m_Allocation  = VK_NULL_HANDLE;
    VkFormat              m_Format      = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits m_SampleCount = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
  };

  // image view
  class ImageView: public HandledObject {
  public:
    ImageView(VkImage image, VkFormat format, VkImageViewType view_type, VkImageAspectFlags aspect_mask);
    ImageView(Handle<Image> image, VkImageViewType view_type, VkImageAspectFlags aspect_mask);
    ImageView(const ImageView& other);
    ~ImageView();
  public:
    inline VkImageView GetImageView() const { return m_ImageView; }
  private:
    void CreateImageView(VkImage image, VkFormat format, VkImageViewType view_type, VkImageAspectFlags aspect_mask);
  private:
    VkImageView m_ImageView = VK_NULL_HANDLE;
  };

  // framebuffer
  class Framebuffer: public HandledObject {
  public:
    Framebuffer(std::vector<Handle<ImageView>> image_views, Handle<Renderpass> renderpass, TUint32 width, TUint32 height);
    Framebuffer(Handle<ImageView> color, Handle<ImageView> depth, Handle<Renderpass> renderpass, TUint32 width, TUint32 height);
    ~Framebuffer();
  public:
    inline VkFramebuffer Get() const { return m_Framebuffer; }
  private:
    void CreateFramebuffer(std::vector<VkImageView> image_views, VkRenderPass renderpass, TUint32 width, TUint32 height);
  private:
    VkFramebuffer m_Framebuffer;

  };

}
