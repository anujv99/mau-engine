#pragma once

#ifdef MAU_OPTIX

#include <optix_types.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <engine/types.h>
#include <engine/utils/singleton.h>

#include "graphics/vulkan-buffers.h"
#include "graphics/vulkan-image.h"
#include "graphics/vulkan-commands.h"

namespace mau {

  class Denoiser: public Singleton<Denoiser> {
    friend class Singleton<Denoiser>;

  private:
    Denoiser();
    ~Denoiser();

  public:
    void AllocateBuffers(TUint32 width, TUint32 height);
    void ImageToBuffers(Handle<CommandBuffer> &cmd, const Handle<Image> &color, const Handle<Image> &albedo, const Handle<Image> &normal, VkImageLayout layout);
    void BufferToImage(Handle<CommandBuffer> &cmd, const Handle<Image> &output, VkImageLayout layout);
    void Denoise(Handle<CommandBuffer> &cmd);

  private:
    void                      Init();
    void                      DestroyBuffers();
    [[nodiscard]] CUdeviceptr CreateCudaBuffer(const Handle<Buffer> &buffer);

  private:
    VkDevice           m_Device = VK_NULL_HANDLE;
    VkPhysicalDevice   m_PhysicalDevice = VK_NULL_HANDLE;
    OptixDeviceContext m_OptixContext = nullptr;
    OptixDenoiser      m_Denoiser = nullptr;

    OptixPixelFormat       m_PixelFormat = OPTIX_PIXEL_FORMAT_FLOAT4;
    TUint32                m_SizeOfPixel = 16u;
    OptixDenoiserAlphaMode m_DenoiserAlphaMode = OPTIX_DENOISER_ALPHA_MODE_COPY;
    OptixDenoiserOptions   m_DenoiserOptions = {};
    OptixDenoiserSizes     m_DenoiserSizes = {};

    glm::uvec2 m_ImageSize = {0u, 0u};

    Handle<Buffer> m_ColorBuffer = nullptr;
    Handle<Buffer> m_AlbedoBuffer = nullptr;
    Handle<Buffer> m_NormalBuffer = nullptr;
    Handle<Buffer> m_OutputBuffer = nullptr;

    CUdeviceptr m_StateBuffer = {};
    CUdeviceptr m_ScratchBuffer = {};
    CUdeviceptr m_Intensity = {};
    CUdeviceptr m_MinRGB = {};
    CUstream    m_CuStream = {};

    CUdeviceptr m_CudaColorBuffer = {};
    CUdeviceptr m_CudaAlbedoBuffer = {};
    CUdeviceptr m_CudaNormalBuffer = {};
    CUdeviceptr m_CudaOutputBuffer = {};
  };

} // namespace mau

#endif // MAU_OPTIX
