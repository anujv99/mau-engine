#pragma once

#include <engine/types.h>

#include "graphics/vulkan-buffers.h"
#include "material.h"

namespace mau {

  class SubMesh {
    friend class Mesh;
  private:
    SubMesh(Handle<VertexBuffer> vertex_buffer, Handle<IndexBuffer> index_buffer, TUint32 index_count, Handle<Material> material);
  public:
    ~SubMesh() = default;
  public:
    inline Handle<VertexBuffer> GetVertexBuffer() const { return m_Vertices; }
    inline Handle<IndexBuffer> GetIndexBuffer() const { return m_Indices; }
    inline TUint32 GetIndexCount() const { return m_IndexCount; }
    inline Handle<Material> GetMaterial() const { return m_Material; }
    inline Handle<AccelerationBuffer> GetAccel() const { return m_Accel; }
  private:
    Handle<VertexBuffer>       m_Vertices   = nullptr;
    Handle<IndexBuffer>        m_Indices    = nullptr;
    Handle<AccelerationBuffer> m_Accel      = nullptr;
    Handle<Material>           m_Material   = nullptr;
    TUint32                    m_IndexCount = 0u;
  };

  class Mesh: public HandledObject {
  public:
    Mesh(const String& filename);
    ~Mesh();
  public:
    inline const Vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }
  private:
    Vector<SubMesh> m_SubMeshes = {};
  };

}
