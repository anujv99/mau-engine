#pragma once

#include <vector>
#include <engine/types.h>
#include "common.h"
#include "vulkan-renderpass.h"

namespace mau {

  // image
  class Image: public HandledObject {
  public:
    Image(TUint32 width, TUint32 height, TUint32 depth, TUint32 mip_levels,
          TUint32 array_layers, VkImageType type, VkSampleCountFlagBits samples,
          VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
    Image(TUint32 width, TUint32 height, VkImage image, VkFormat format,
          VkSampleCountFlagBits samples);
    ~Image();

  public:
    inline VkImage               GetImage() const { return m_Image; }
    inline VkFormat              GetFormat() const { return m_Format; }
    inline VkSampleCountFlagBits GetSamples() const { return m_SampleCount; }
    inline TUint32               GetWidth() const { return m_Width; }
    inline TUint32               GetHeight() const { return m_Height; }

  private:
    VkImage               m_Image = VK_NULL_HANDLE;
    VmaAllocation         m_Allocation = VK_NULL_HANDLE;
    VkFormat              m_Format = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits m_SampleCount = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
    TUint32               m_Width = 0u;
    TUint32               m_Height = 0u;
  };

  void TransitionImageLayout(Handle<CommandBuffer> cmd, Handle<Image> image,
                             VkImageLayout old_layout,
                             VkImageLayout new_layout);

  // image view
  class ImageView: public HandledObject {
  public:
    ImageView(VkImage image, VkFormat format, VkImageViewType view_type,
              VkImageAspectFlags aspect_mask);
    ImageView(Handle<Image> image, VkImageViewType view_type,
              VkImageAspectFlags aspect_mask);
    ImageView(const ImageView &other);
    ~ImageView();

  public:
    inline VkImageView GetImageView() const { return m_ImageView; }

  private:
    void CreateImageView(VkImage image, VkFormat format,
                         VkImageViewType    view_type,
                         VkImageAspectFlags aspect_mask);

  private:
    VkImageView m_ImageView = VK_NULL_HANDLE;
  };

  // framebuffer
  class Framebuffer: public HandledObject {
  public:
    Framebuffer(std::vector<Handle<ImageView>> image_views,
                Handle<Renderpass> renderpass, TUint32 width, TUint32 height);
    Framebuffer(Handle<ImageView> color, Handle<ImageView> depth,
                Handle<Renderpass> renderpass, TUint32 width, TUint32 height);
    ~Framebuffer();

  public:
    inline VkFramebuffer Get() const { return m_Framebuffer; }

  private:
    void CreateFramebuffer(std::vector<VkImageView> image_views,
                           VkRenderPass renderpass, TUint32 width,
                           TUint32 height);

  private:
    VkFramebuffer m_Framebuffer;
  };

  // image sampler
  class Sampler: public HandledObject {
  public:
    Sampler();
    ~Sampler();

  public:
    inline VkSampler Get() const { return m_Sampler; }

  private:
    VkSampler m_Sampler = VK_NULL_HANDLE;
  };

  // texture 2d
  class Texture: public HandledObject {
  public:
    Texture(const String &image_path);
    ~Texture() = default;

  public:
    VkDescriptorImageInfo GetDescriptorInfo() const;

  private:
    Handle<Image>     m_Image = nullptr;
    Handle<ImageView> m_ImageView = nullptr;
    Sampler           m_Sampler;
  };

} // namespace mau
