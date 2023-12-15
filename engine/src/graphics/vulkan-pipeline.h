#pragma once

#include <memory>
#include "common.h"
#include "vulkan-shaders.h"
#include "vulkan-renderpass.h"
#include "vulkan-push-constant.h"
#include "vulkan-buffers.h"

namespace mau {

  struct RTPipelineCreateInfo {
    Handle<RTClosestHitShader>    ClosestHit;
    Handle<RTRayGenShader>        RayGen;
    Handle<RTMissShader>          Miss;
    Handle<PushConstantBase>      PushConstant;
    Vector<VkDescriptorSetLayout> DescriptorLayouts;
  };

  struct RTSBTRegion {
    VkStridedDeviceAddressRegionKHR RayGen;
    VkStridedDeviceAddressRegionKHR RayMiss;
    VkStridedDeviceAddressRegionKHR RayClosestHit;
    VkStridedDeviceAddressRegionKHR RayCall;
  };

  class InputLayout {
  public:
    InputLayout();
    ~InputLayout();

  public:
    void AddBindingDesc(TUint32 binding, TUint32 stride);
    void AddAttributeDesc(TUint32 location, TUint32 binding, VkFormat format,
                          TUint32 offset);

    inline const std::vector<VkVertexInputBindingDescription> &
    GetBindingDesc() const {
      return m_BindingDesc;
    }
    inline const std::vector<VkVertexInputAttributeDescription> &
    GetAttributeDesc() const {
      return m_AttributeDesc;
    }

  private:
    std::vector<VkVertexInputBindingDescription>   m_BindingDesc = {};
    std::vector<VkVertexInputAttributeDescription> m_AttributeDesc = {};
  };

  class Pipeline: public HandledObject {
  public:
    Pipeline(Handle<VertexShader>   vertex_shader,
             Handle<FragmentShader> fragment_shader,
             Handle<Renderpass> renderpass, const InputLayout &input_layout,
             Handle<PushConstantBase>                  push_constant = nullptr,
             const std::vector<VkDescriptorSetLayout> &descriptor_layouts = {},
             const VkSampleCountFlagBits &sample_count = VK_SAMPLE_COUNT_1_BIT);
    ~Pipeline();

  public:
    inline VkPipeline       Get() const { return m_Pipeline; }
    inline VkPipelineLayout GetLayout() const { return m_PipelineLayout; }

  private:
    VkPipeline       m_Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
  };

  class RTPipeline: public HandledObject {
  public:
    RTPipeline(const RTPipelineCreateInfo &create_info);
    ~RTPipeline();

  public:
    inline VkPipeline       Get() const { return m_Pipeline; }
    inline VkPipelineLayout GetLayout() const { return m_PipelineLayout; }

    RTSBTRegion GetSBTRegion() const;

  private:
    void CreateShaderBindingTable();

  private:
    VkPipeline       m_Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

    Handle<Buffer>                  m_SBTBuffer = nullptr;
    VkStridedDeviceAddressRegionKHR m_RayGenRegion = {};
    VkStridedDeviceAddressRegionKHR m_RayMissRegion = {};
    VkStridedDeviceAddressRegionKHR m_RayClosestHitRegion = {};
    VkStridedDeviceAddressRegionKHR m_RayCallRegion = {};
  };

} // namespace mau
