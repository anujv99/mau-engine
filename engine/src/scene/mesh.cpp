#include "mesh.h"

#include <functional>
#include <filesystem>
#include <engine/log.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include "graphics/vulkan-bindless.h"
#include "graphics/vulkan-features.h"

namespace mau {

  struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex;
  };

  SubMesh::SubMesh(Handle<VertexBuffer> vertex_buffer,
                   Handle<IndexBuffer> index_buffer, TUint32 index_count,
                   Handle<Material> material)
      : m_Vertices(vertex_buffer), m_Indices(index_buffer),
        m_IndexCount(index_count), m_Material(material) {

    if (!VulkanFeatures::IsRtEnabled())
      return;

    RTObjectDesc desc = {
        .VertexBuffer = vertex_buffer->GetDeviceAddress(),
        .IndexBuffer = index_buffer->GetDeviceAddress(),
        .Material = material->GetMaterialHandle(),

        .padding = {0, 0, 0},
    };
    m_RTDescHandle = VulkanBindless::Ref().AddRTObject(desc);

    AccelerationBufferCreateInfo create_info = {
        .Vertices = vertex_buffer,
        .Indices = index_buffer,
        .VertexSize = sizeof(Vertex),
        .PositionOffset = offsetof(Vertex, pos),
        .VertexCount =
            static_cast<TUint32>(vertex_buffer->GetSize() / sizeof(Vertex)),
        .IndexCount = index_count,
        .CustomIndex = m_RTDescHandle,
    };

    m_Accel = make_handle<BottomLevelAS>(create_info);
  }

  Mesh::Mesh(const String &filename) {
    Assimp::Importer importer;
    const aiScene   *scene = importer.ReadFile(
        filename, aiProcess_Triangulate | aiProcess_PreTransformVertices |
                      aiProcess_RemoveRedundantMaterials);

    if (scene == nullptr) {
      LOG_ERROR("failed to load mesh %s [reason: %s]", filename.c_str(),
                importer.GetErrorString());
      return;
    }

    const String directory =
        std::filesystem::path(filename).parent_path().string();

    struct VertexData {
      Vector<Vertex>  vertices = {};
      Vector<TUint32> indices = {};
    };

    UnorderedMap<TUint32, VertexData> submeshes = {};

    auto process_mesh = [&](aiMesh *mesh) -> void {
      const TUint32 material_index = mesh->mMaterialIndex;

      if (!submeshes.contains(material_index)) {
        submeshes.insert(std::make_pair(material_index, VertexData{}));
      }

      VertexData   &data = submeshes.at(material_index);
      const TUint32 vertex_offset = static_cast<TUint32>(data.vertices.size());

      for (TUint32 i = 0; i < mesh->mNumVertices; i++) {
        Vertex vert;
        vert.pos.x = mesh->mVertices[i].x;
        vert.pos.y = mesh->mVertices[i].y;
        vert.pos.z = mesh->mVertices[i].z;

        vert.normal.x = mesh->mNormals[i].x;
        vert.normal.y = mesh->mNormals[i].y;
        vert.normal.z = mesh->mNormals[i].z;

        if (mesh->mTextureCoords[0]) {
          vert.tex.x = mesh->mTextureCoords[0][i].x;
          vert.tex.y = 1.0f - mesh->mTextureCoords[0][i].y;
        }

        data.vertices.push_back(vert);
      }

      for (TUint32 i = 0; i < mesh->mNumFaces; i++) {
        aiFace *face = &(mesh->mFaces[i]);
        for (TUint32 j = 0; j < face->mNumIndices; j++) {
          data.indices.push_back(vertex_offset + face->mIndices[j]);
        }
      }
    };

    std::function<void(aiNode *)> process_node = [&](aiNode *node) -> void {
      for (TUint32 i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh);
      }

      for (TUint32 i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i]);
      }
    };

    process_node(scene->mRootNode);

    Vector<Handle<BottomLevelAS>> blases = {};

    for (const auto &[material_index, vertex_data] : submeshes) {
      const Vector<Vertex>  &vertices = vertex_data.vertices;
      const Vector<TUint32> &indices = vertex_data.indices;

      Handle<VertexBuffer> vertex_buffer = make_handle<VertexBuffer>(
          vertices.size() * sizeof(vertices[0]), vertices.data());
      Handle<IndexBuffer> index_buffer = make_handle<IndexBuffer>(
          indices.size() * sizeof(indices[0]), indices.data());
      TUint32          index_count = static_cast<TUint32>(indices.size());
      Handle<Material> material = nullptr;

      if (material_index >= 0) {
        aiMaterial        *ai_material = scene->mMaterials[material_index];
        MaterialCreateInfo create_info = {};

        const TUint32 diffuse_map_count =
            ai_material->GetTextureCount(aiTextureType_DIFFUSE);
        const TUint32 normal_map_count =
            ai_material->GetTextureCount(aiTextureType_NORMALS);

        if (diffuse_map_count > 0) {
          aiString texture_path;
          ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path);
          create_info.DiffuseMap = directory + "/" + texture_path.C_Str();
        }

        if (normal_map_count > 0) {
          aiString texture_path;
          ai_material->GetTexture(aiTextureType_NORMALS, 0, &texture_path);
          create_info.NormalMap = directory + "/" + texture_path.C_Str();
        }

        material = make_handle<Material>(create_info);
      }

      SubMesh submesh(vertex_buffer, index_buffer, index_count, material);
      m_SubMeshes.push_back(submesh);
      blases.push_back(submesh.GetAccel());
    }

    if (VulkanFeatures::IsRtEnabled()) {
      m_TLAS = make_handle<AccelerationBuffer>(blases);
      VulkanBindless::Ref().AddAccelerationStructure(m_TLAS);
    }
  }

  Mesh::~Mesh() { m_SubMeshes.clear(); }

} // namespace mau
