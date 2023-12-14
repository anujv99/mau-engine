#include "mesh.h"

#include <functional>
#include <engine/log.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

namespace mau {

  struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex;
  };

  Mesh::Mesh(const String& filename) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate);

    if (scene == nullptr) {
      LOG_ERROR("failed to load mesh %s [reason: %s]", filename.c_str(), importer.GetErrorString());
      return;
    }

    std::vector<Vertex> vertices = {};
    std::vector<TUint32> indices = {};

    auto process_mesh = [&](aiMesh* mesh, TUint32 index_offset) -> void {
      for (TUint32 i = 0; i < mesh->mNumVertices; i++) {
        Vertex vert;
        vert.pos.x = mesh->mVertices[i].x;
        vert.pos.y = mesh->mVertices[i].y; 
        vert.pos.z = mesh->mVertices[i].z;

        vert.normal.x = mesh->mNormals[i].x;
        vert.normal.y = mesh->mNormals[i].y;
        vert.normal.z = mesh->mNormals[i].z;

        vert.tex.x = mesh->mTextureCoords[0][i].x;
        vert.tex.y = mesh->mTextureCoords[0][i].y;

        vertices.push_back(vert);
      }

      for (TUint32 i = 0; i < mesh->mNumFaces; i++) {
        aiFace* face = &(mesh->mFaces[i]);
        for (TUint32 j = 0; j < face->mNumIndices; j++) {
          indices.push_back(index_offset + face->mIndices[j]);
        }
      }
    };

    std::function<void(aiNode*)> process_node = [&](aiNode* node) -> void {
      for (TUint32 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh, static_cast<TUint32>(vertices.size()));
      }

      for (TUint32 i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i]);
      }
    };

    process_node(scene->mRootNode);

    m_Vertices = make_handle<VertexBuffer>(vertices.size() * sizeof(vertices[0]), vertices.data());
    m_Indices = make_handle<IndexBuffer>(indices.size() * sizeof(indices[0]), indices.data());
    m_IndexCount = static_cast<TUint32>(indices.size());
  }

  Mesh::~Mesh() {
    m_Vertices = nullptr;
    m_Indices = nullptr;
  }

}
