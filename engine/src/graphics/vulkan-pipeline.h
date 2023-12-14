#pragma once

#include <memory>
#include "common.h"
#include "vulkan-shaders.h"
#include "vulkan-renderpass.h"
#include "vulkan-push-constant.h"

namespace mau {

  class InputLayout {
  public:
    InputLayout();
    ~InputLayout();
  public:
    void AddBindingDesc(TUint32 binding, TUint32 stride);
    void AddAttributeDesc(TUint32 location, TUint32 binding, VkFormat format, TUint32 offset);

    inline const std::vector<VkVertexInputBindingDescription>& GetBindingDesc() const { return m_BindingDesc; }
    inline const std::vector<VkVertexInputAttributeDescription>& GetAttributeDesc() const { return m_AttributeDesc; }
  private:
    std::vector<VkVertexInputBindingDescription>   m_BindingDesc;
    std::vector<VkVertexInputAttributeDescription> m_AttributeDesc;
  };

  class Pipeline: public HandledObject {
  public:
    Pipeline(Handle<VertexShader> vertex_shader, Handle<FragmentShader> fragment_shader, Handle<Renderpass> renderpass, const InputLayout& input_layout, Handle<PushConstantBase> push_constant = nullptr);
    ~Pipeline();
  public:
    inline VkPipeline Get() const { return m_Pipeline; }
    inline VkPipelineLayout GetLayout() const { return m_PipelineLayout; }
  private:
    VkPipeline       m_Pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
  };

}
