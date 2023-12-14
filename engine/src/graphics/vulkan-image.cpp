#include "vulkan-image.h"

#include <engine/assert.h>

#include "vulkan-state.h"
#include "vulkan-buffers.h"
#include "../loader/image-loader.h"

namespace mau {
  
  void TransitionImageLayout(Handle<CommandBuffer> cmd, Handle<Image> image, VkImageLayout old_layout, VkImageLayout new_layout) {
    VkImageSubresourceRange subresource = {};
    subresource.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.baseMipLevel            = 0u;
    subresource.levelCount              = 1u;
    subresource.baseArrayLayer          = 0u;
    subresource.layerCount              = 1u;

    VkImageMemoryBarrier barrier = {};
    barrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext                = nullptr;
    barrier.srcAccessMask        = VK_ACCESS_NONE;
    barrier.dstAccessMask        = VK_ACCESS_NONE;
    barrier.oldLayout            = old_layout;
    barrier.newLayout            = new_layout;
    barrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                = image->GetImage();
    barrier.subresourceRange     = subresource;

    switch (old_layout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;
    default:
      break;
    }

    switch (new_layout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      if (barrier.srcAccessMask == VK_ACCESS_NONE) {
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
      }
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    default:
      break;
    }

    vkCmdPipelineBarrier(cmd->Get(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
  }

  void CopyBufferToImage(Handle<CommandBuffer> cmd, Buffer* buffer, Handle<Image> image, TUint32 width, TUint32 height) {
    VkExtent3D extent = {
      .width  = width,
      .height = height,
      .depth  = 1u,
    };

    VkOffset3D offset = {
      .x = 0u,
      .y = 0u,
      .z = 0u,
    };

    VkImageSubresourceLayers subresource = {
      .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel       = 0u,
      .baseArrayLayer = 0u,
      .layerCount     = 1u,
    };

    VkBufferImageCopy region = {
      .bufferOffset      = 0u,
      .bufferRowLength   = 0u,
      .bufferImageHeight = 0u,
      .imageSubresource  = subresource,
      .imageOffset       = offset,
      .imageExtent       = extent,
    };

    vkCmdCopyBufferToImage(cmd->Get(), buffer->Get(), image->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  }

  void UploadUsingStaging(Handle<Image> image, const void* data, TUint32 width, TUint32 height, TUint32 channels) {
    ASSERT(image && data);

    TUint64 image_size = static_cast<TUint64>(width) * static_cast<TUint64>(height) * static_cast<TUint64>(channels);

    Buffer staging_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* buffer_mem = staging_buffer.Map();
    memcpy(buffer_mem, data, image_size);
    staging_buffer.UnMap();

    Handle<CommandBuffer> cmd = VulkanState::Ref().GetCommandPool(VK_QUEUE_TRANSFER_BIT)->AllocateCommandBuffers(1)[0];

    cmd->Begin();
    TransitionImageLayout(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(cmd, &staging_buffer, image, width, height);
    TransitionImageLayout(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    cmd->End();

    Handle<VulkanQueue> transfer_queue = VulkanState::Ref().GetDeviceHandle()->GetTransferQueue();
    transfer_queue->Submit(cmd);
    transfer_queue->WaitIdle();
  }

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

  // framebuffer
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

  // sampler
  Sampler::Sampler() {
    VkSamplerCreateInfo create_info     = {};
    create_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.pNext                   = nullptr;
    create_info.flags                   = 0u;
    create_info.magFilter               = VK_FILTER_LINEAR;
    create_info.minFilter               = VK_FILTER_LINEAR;
    create_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    create_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    create_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    create_info.mipLodBias              = 0.0f;
    create_info.anisotropyEnable        = VK_TRUE;
    create_info.maxAnisotropy           = VulkanState::Ref().GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
    create_info.compareEnable           = VK_FALSE;
    create_info.compareOp               = VK_COMPARE_OP_ALWAYS;
    create_info.minLod                  = 0.0f;
    create_info.maxLod                  = 0.0f;
    create_info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    create_info.unnormalizedCoordinates = VK_FALSE;

    VK_CALL(vkCreateSampler(VulkanState::Ref().GetDevice(), &create_info, nullptr, &m_Sampler));
  }

  Sampler::~Sampler() {
    vkDestroySampler(VulkanState::Ref().GetDevice(), m_Sampler, nullptr);
  }

  // texture
  Texture::Texture(const String& image_path) {
    RawImage raw_image(image_path);

    if (raw_image.Data) {
      m_Image = make_handle<Image>(
        raw_image.Width, raw_image.Height,
        1u, 1u, 1u,
        VK_IMAGE_TYPE_2D,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
      );
      UploadUsingStaging(m_Image, raw_image.Data, raw_image.Width, raw_image.Height, 4u);
      m_ImageView = make_handle<ImageView>(m_Image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
    }
  }

  VkDescriptorImageInfo Texture::GetDescriptorInfo() const {
    VkDescriptorImageInfo descriptor_info = {
      .sampler     = m_Sampler.Get(),
      .imageView   = m_ImageView->GetImageView(),
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    return descriptor_info;
  }

}
