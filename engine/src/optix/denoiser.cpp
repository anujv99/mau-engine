#include "denoiser.h"
#include "engine/types.h"

#ifdef MAU_OPTIX

#include <optix.h>
#include <optix_function_table_definition.h>
#include <optix_stubs.h>

#include <engine/log.h>
#include "graphics/vulkan-state.h"

#define CUDA_CALL(call)                                                                                                                                                                                \
  do {                                                                                                                                                                                                 \
    const cudaError_t status = call;                                                                                                                                                                   \
    if (status != cudaSuccess) {                                                                                                                                                                       \
      const char *err = cudaGetErrorName(status);                                                                                                                                                      \
      LOG_ERROR("CUDA error: %s", err);                                                                                                                                                                \
    }                                                                                                                                                                                                  \
  } while (false)

#define OPTIX_CALL(call)                                                                                                                                                                               \
  do {                                                                                                                                                                                                 \
    const OptixResult status = call;                                                                                                                                                                   \
    if (status != OPTIX_SUCCESS) {                                                                                                                                                                     \
      const char *err = optixGetErrorName(status);                                                                                                                                                     \
      LOG_ERROR("Optix error: %s", err);                                                                                                                                                               \
    }                                                                                                                                                                                                  \
  } while (false)

namespace mau {

  static void optix_context_log_cb(unsigned int level, const char *tag, const char *message, void *) {
    switch (level) {
    case 1:
      LOG_FATAL("[OPTIX] %s: %s", tag, message);
      break;
    case 2:
      LOG_ERROR("[OPTIX] %s: %s", tag, message);
      break;
    case 3:
      LOG_WARN("[OPTIX] %s: %s", tag, message);
      break;
    case 4:
      LOG_TRACE("[OPTIX] %s: %s", tag, message);
      break;
    default:
      LOG_ERROR("[OPTIX] %s: %s", tag, message);
      break;
    }
  }

  Denoiser::Denoiser() {
    m_Device = VulkanState::Ref().GetDevice();
    m_PhysicalDevice = VulkanState::Ref().GetPhysicalDevice();
    Init();
  }

  Denoiser::~Denoiser() { }

  void Denoiser::AllocateBuffers(TUint32 width, TUint32 height) {
    m_ImageSize = {width, height};

    DestroyBuffers();

    TUint64            buffer_size = width * height * 4 * sizeof(float);
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    {
      // color
      m_ColorBuffer = make_handle<Buffer>(buffer_size, usage, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
      m_CudaColorBuffer = CreateCudaBuffer(m_ColorBuffer);
    }

    {
      // albedo
      m_AlbedoBuffer = make_handle<Buffer>(buffer_size, usage, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
      m_CudaAlbedoBuffer = CreateCudaBuffer(m_AlbedoBuffer);
    }

    {
      // normal
      m_NormalBuffer = make_handle<Buffer>(buffer_size, usage, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
      m_CudaNormalBuffer = CreateCudaBuffer(m_NormalBuffer);
    }

    {
      // output
      m_OutputBuffer = make_handle<Buffer>(buffer_size, usage, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
      m_CudaOutputBuffer = CreateCudaBuffer(m_OutputBuffer);
    }

    OPTIX_CALL(optixDenoiserComputeMemoryResources(m_Denoiser, m_ImageSize.x, m_ImageSize.y, &m_DenoiserSizes));

    CUDA_CALL(cudaMalloc(reinterpret_cast<void **>(&m_StateBuffer), m_DenoiserSizes.stateSizeInBytes));
    CUDA_CALL(cudaMalloc(reinterpret_cast<void **>(&m_ScratchBuffer), m_DenoiserSizes.withoutOverlapScratchSizeInBytes));
    CUDA_CALL(cudaMalloc(reinterpret_cast<void **>(&m_MinRGB), 4 * sizeof(float)));
    CUDA_CALL(cudaMalloc(reinterpret_cast<void **>(&m_Intensity), sizeof(float)));

    OPTIX_CALL(
        optixDenoiserSetup(m_Denoiser, m_CuStream, m_ImageSize.x, m_ImageSize.y, m_StateBuffer, m_DenoiserSizes.stateSizeInBytes, m_ScratchBuffer, m_DenoiserSizes.withoutOverlapScratchSizeInBytes));
  }

  void Denoiser::ImageToBuffers(Handle<CommandBuffer> &cmd, const Handle<Image> &color, const Handle<Image> &albedo, const Handle<Image> &normal, VkImageLayout layout) {
    VkBufferImageCopy copy_region = {
        .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1u},
        .imageExtent = {.width = m_ImageSize.x, .height = m_ImageSize.y, .depth = 1u},
    };

    TransitionImageLayout(cmd, color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkCmdCopyImageToBuffer(cmd->Get(), color->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_ColorBuffer->Get(), 1u, &copy_region);
    TransitionImageLayout(cmd, color, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, layout);

    TransitionImageLayout(cmd, albedo, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkCmdCopyImageToBuffer(cmd->Get(), albedo->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_AlbedoBuffer->Get(), 1u, &copy_region);
    TransitionImageLayout(cmd, albedo, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

    TransitionImageLayout(cmd, normal, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkCmdCopyImageToBuffer(cmd->Get(), normal->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_NormalBuffer->Get(), 1u, &copy_region);
    TransitionImageLayout(cmd, normal, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
  }

  void Denoiser::BufferToImage(Handle<CommandBuffer> &cmd, const Handle<Image> &output, VkImageLayout layout) {
    VkBufferImageCopy copy_region = {
        .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1u},
        .imageExtent = {.width = m_ImageSize.x, .height = m_ImageSize.y, .depth = 1u},
    };

    TransitionImageLayout(cmd, output, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vkCmdCopyBufferToImage(cmd->Get(), m_OutputBuffer->Get(), output->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &copy_region);
    TransitionImageLayout(cmd, output, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layout);
  }

  void Denoiser::Denoise(Handle<CommandBuffer> &cmd) {
    const OptixPixelFormat &pixel_format = m_PixelFormat;
    const TUint32          &size_of_pixel = m_SizeOfPixel;
    const TUint32           row_stride_in_bytes = m_ImageSize.x * size_of_pixel;

    // clang-format off
    OptixDenoiserLayer layer = {
      .input = {
        .data = m_CudaColorBuffer,
        .width = m_ImageSize.x,
        .height = m_ImageSize.y,
        .rowStrideInBytes = row_stride_in_bytes,
        .pixelStrideInBytes = size_of_pixel,
        .format = pixel_format,
      },

      .output = {
        .data = m_CudaOutputBuffer,
        .width = m_ImageSize.x,
        .height = m_ImageSize.y,
        .rowStrideInBytes = row_stride_in_bytes,
        .pixelStrideInBytes = sizeof(float) * 4u,
        .format = pixel_format,
      },
    };

    OptixDenoiserGuideLayer guide_layer = {
      .albedo = {
        .data = m_CudaAlbedoBuffer,
        .width = m_ImageSize.x,
        .height = m_ImageSize.y,
        .rowStrideInBytes = row_stride_in_bytes,
        .pixelStrideInBytes = size_of_pixel,
        .format = pixel_format,
      },

      .normal = {
        .data = m_CudaNormalBuffer,
        .width = m_ImageSize.x,
        .height = m_ImageSize.y,
        .rowStrideInBytes = row_stride_in_bytes,
        .pixelStrideInBytes = size_of_pixel,
        .format = pixel_format,
      },
    };
    // clang-format on

    OPTIX_CALL(optixDenoiserComputeIntensity(m_Denoiser, m_CuStream, &layer.input, m_Intensity, m_MinRGB, m_DenoiserSizes.stateSizeInBytes));

    OptixDenoiserParams params = {
        .hdrIntensity = m_Intensity,
        .blendFactor = 0.0f,
    };

    OPTIX_CALL(optixDenoiserInvoke(m_Denoiser, m_CuStream, &params, m_StateBuffer, m_DenoiserSizes.stateSizeInBytes, &guide_layer, &layer, 1, 0, 0, m_ScratchBuffer,
                                   m_DenoiserSizes.withoutOverlapScratchSizeInBytes));

    CUDA_CALL(cudaDeviceSynchronize());
    CUDA_CALL(cudaStreamSynchronize(m_CuStream));
  }

  CUdeviceptr Denoiser::CreateCudaBuffer(const Handle<Buffer> &buffer) {
    int fd = -1;

    VkMemoryGetFdInfoKHR fd_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
        .pNext = nullptr,
        .memory = buffer->GetDeviceMemory(),
        .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR,
    };
    vkGetMemoryFdKHR(m_Device, &fd_info, &fd);

    VkMemoryRequirements memory_requirements = {};
    vkGetBufferMemoryRequirements(m_Device, buffer->Get(), &memory_requirements);

    cudaExternalMemoryHandleDesc cuda_ext_mem_handle_desc = {
        .type = cudaExternalMemoryHandleTypeOpaqueFd,
        .handle = {.fd = fd},
        .size = memory_requirements.size,
        .flags = 0u,
    };

    cudaExternalMemory_t cuda_ext_mem = nullptr;
    CUDA_CALL(cudaImportExternalMemory(&cuda_ext_mem, &cuda_ext_mem_handle_desc));
    cuda_ext_mem_handle_desc.handle.fd = -1;

    cudaExternalMemoryBufferDesc cuda_ext_mem_buffer_desc = {
        .offset = 0u,
        .size = memory_requirements.size,
        .flags = 0u,
    };

    void *cuda_ptr = nullptr;
    CUDA_CALL(cudaExternalMemoryGetMappedBuffer(&cuda_ptr, cuda_ext_mem, &cuda_ext_mem_buffer_desc));
    return reinterpret_cast<CUdeviceptr>(cuda_ptr);
  }

  void Denoiser::Init() {
    CUDA_CALL(cudaFree(nullptr));

    CUcontext cu_ctx = nullptr;
    OPTIX_CALL(optixInit());

    OptixDeviceContextOptions options = {};
    options.logCallbackFunction = &optix_context_log_cb;
    options.logCallbackLevel = 4;
    OPTIX_CALL(optixDeviceContextCreate(cu_ctx, &options, &m_OptixContext));
    OPTIX_CALL(optixDeviceContextSetLogCallback(m_OptixContext, &optix_context_log_cb, nullptr, 4));

    const OptixDenoiserOptions denoiser_options = {
        .guideAlbedo = 1u,
        .guideNormal = 1u,
        .denoiseAlpha = m_DenoiserAlphaMode,
    };
    m_DenoiserOptions = denoiser_options;

    const OptixDenoiserModelKind model_kind = OPTIX_DENOISER_MODEL_KIND_AOV;
    OPTIX_CALL(optixDenoiserCreate(m_OptixContext, model_kind, &m_DenoiserOptions, &m_Denoiser));

    LOG_INFO("OptiX denoiser created");
  }

  void Denoiser::DestroyBuffers() {
    if (m_ColorBuffer)
      m_ColorBuffer = nullptr;

    if (m_AlbedoBuffer)
      m_AlbedoBuffer = nullptr;

    if (m_NormalBuffer)
      m_NormalBuffer = nullptr;

    if (m_OutputBuffer)
      m_OutputBuffer = nullptr;

    if (m_StateBuffer) {
      CUDA_CALL(cudaFree(reinterpret_cast<void *>(m_StateBuffer)));
      m_StateBuffer = 0u;
    }

    if (m_ScratchBuffer) {
      CUDA_CALL(cudaFree(reinterpret_cast<void *>(m_ScratchBuffer)));
      m_ScratchBuffer = 0u;
    }

    if (m_Intensity) {
      CUDA_CALL(cudaFree(reinterpret_cast<void *>(m_Intensity)));
      m_Intensity = 0u;
    }

    if (m_MinRGB) {
      CUDA_CALL(cudaFree(reinterpret_cast<void *>(m_MinRGB)));
      m_MinRGB = 0u;
    }

    if (m_CudaColorBuffer) {
      CUDA_CALL(cudaFree(reinterpret_cast<void *>(m_CudaColorBuffer)));
      m_CudaColorBuffer = 0u;
    }

    if (m_CudaOutputBuffer) {
      CUDA_CALL(cudaFree(reinterpret_cast<void *>(m_CudaOutputBuffer)));
      m_CudaOutputBuffer = 0u;
    }
  }

} // namespace mau

#endif // MAU_OPTIX
