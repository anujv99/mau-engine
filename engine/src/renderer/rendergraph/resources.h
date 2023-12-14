#pragma once

#include <engine/assert.h>
#include <engine/utils/handle.h>
#include "graphics/vulkan-image.h"
#include "graphics/vulkan-buffers.h"

namespace mau {

  enum class ResourceType {
    IMAGE,
    TEXTURE,
    BUFFER,
  };

  class Resource: public HandledObject {
  public:
    Resource(ResourceType type): m_Type(type) { }
    virtual ~Resource() { }
  protected:
    const ResourceType m_Type;
  };

  class ImageResource: public Resource {
  public:
    ImageResource(Handle<Image> image, Handle<ImageView> image_view): Resource(ResourceType::IMAGE), m_Image(image), m_ImageView(image_view) { ASSERT(image && image_view); }
    ~ImageResource() = default;
  public:
    inline Handle<Image> GetImage() const { return m_Image; }
    inline Handle<ImageView> GetImageView() const { return m_ImageView; }
  private:
    Handle<Image>     m_Image     = nullptr;
    Handle<ImageView> m_ImageView = nullptr;
  };

  class TextureResource: public Resource {
  public:
    TextureResource(Handle<Texture> texture): Resource(ResourceType::TEXTURE), m_Texture(texture) { ASSERT(texture); }
    ~TextureResource() = default;
  private:
    Handle<Texture> m_Texture = nullptr;
  };

  class BufferResource: public Resource {
  public:
    BufferResource(Handle<UniformBuffer> buffer): Resource(ResourceType::BUFFER), m_Buffer(buffer) { ASSERT(buffer); }
    ~BufferResource() = default;
  private:
    Handle<UniformBuffer> m_Buffer = nullptr;
  };

}
