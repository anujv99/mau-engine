#pragma once

#include <engine/types.h>
#include "../graphics/vulkan-buffers.h"

namespace mau {

  class Mesh: public HandledObject {
  public:
    Mesh(const String& filename);
    ~Mesh();
  public:
    inline Handle<VertexBuffer> GetVertexBuffer() const { return m_Vertices; }
    inline Handle<IndexBuffer> GetIndexBuffer() const { return m_Indices; }
    inline TUint32 GetIndexCount() const { return m_IndexCount; }
  private:
    Handle<VertexBuffer> m_Vertices   = nullptr;
    Handle<IndexBuffer>  m_Indices    = nullptr;
    TUint32              m_IndexCount = 0u;
  };

}
