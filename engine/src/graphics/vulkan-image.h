#pragma once

#include "common.h"

namespace mau {

  // TODO: add image class

  // image view
  class ImageView {
  public:
    ImageView(VkImage image, VkFormat format, VkImageViewType view_type, VkDevice device);
    ImageView(const ImageView& other);
    ~ImageView();
  private:
    VkDevice    m_Device    = VK_NULL_HANDLE;
    VkImageView m_ImageView = VK_NULL_HANDLE;
  };

}
