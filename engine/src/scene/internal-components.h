#include "mesh.h"

namespace mau {

  struct MeshComponent {
    Handle<Mesh> MeshObject = nullptr;

    MeshComponent(const Handle<Mesh>& mesh): MeshObject(mesh) { }
    MeshComponent() = default;
    ~MeshComponent() { MeshObject = nullptr; }
  };
  
}
