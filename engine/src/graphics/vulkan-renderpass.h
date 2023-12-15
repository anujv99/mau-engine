#pragma once

#include <vector>
#include <memory>
#include <engine/types.h>
#include <glm/glm.hpp>
#include "common.h"
#include "vulkan-commands.h"

namespace mau {

  class Framebuffer;

  struct LoadStoreOp {
    VkAttachmentLoadOp  LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkAttachmentLoadOp  StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp StencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  };

  class Renderpass: public HandledObject {
  public:
    Renderpass();
    ~Renderpass();

  public:
    void AddColorAttachment(VkFormat format, VkSampleCountFlagBits samples,
                            LoadStoreOp op, VkImageLayout initial_layout,
                            VkImageLayout final_layout);
    void SetDepthAttachment(VkFormat format, VkSampleCountFlagBits samples,
                            LoadStoreOp op, VkImageLayout initial_layout,
                            VkImageLayout final_layout);
    void SetResolveAttachment(VkFormat format, VkSampleCountFlagBits samples,
                              LoadStoreOp op, VkImageLayout initial_layout,
                              VkImageLayout final_layout);

    void Build(VkPipelineBindPoint  bind_point,
               VkPipelineStageFlags src_stage_mask,
               VkPipelineStageFlags dst_stage_mask,
               VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask);
    void Begin(Handle<CommandBuffer> cmd, Handle<Framebuffer> framebuffer,
               VkRect2D area);
    void End(Handle<CommandBuffer> cmd);

    inline VkRenderPass Get() const { return m_Renderpass; }

  private:
    VkRenderPass m_Renderpass = VK_NULL_HANDLE;
    TUint32      m_AttachmentCount = 0u;
    bool         m_HasDepthAttachment = false;
    bool         m_HasResolveAttachment = false;

    std::vector<VkAttachmentDescription> m_Attachments = {};
    std::vector<VkClearValue>            m_ClearValues = {};
    std::vector<VkAttachmentReference>   m_ColorAttachmentsRef = {};
    VkAttachmentReference                m_DepthAttachmentRef = {};
    VkAttachmentReference                m_ResolveAttachmentRef = {};
  };

} // namespace mau
