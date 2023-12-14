#include "vulkan-renderpass.h"

#include "vulkan-state.h"

#include <engine/assert.h>

namespace mau {

  struct AttachmentDescription {
    VkAttachmentDescription Description = {};
    VkAttachmentReference   Reference   = {};
  };

  AttachmentDescription build_attachment_description(VkFormat format, VkSampleCountFlagBits samples, LoadStoreOp op, VkImageLayout initial_layout, VkImageLayout final_layout, VkImageLayout subpass_layout, TUint32 attachment_index) {
    AttachmentDescription desc = {};

    desc.Description.flags          = {};
    desc.Description.format         = format;
    desc.Description.samples        = samples;
    desc.Description.loadOp         = op.LoadOp;
    desc.Description.storeOp        = op.StoreOp;
    desc.Description.stencilLoadOp  = op.StencilLoadOp;
    desc.Description.stencilStoreOp = op.StencilStoreOp;
    desc.Description.initialLayout  = initial_layout;
    desc.Description.finalLayout    = final_layout;
    desc.Reference.attachment       = attachment_index;
    desc.Reference.layout           = subpass_layout;

    return desc;
  }

  Renderpass::Renderpass() {}

  Renderpass::~Renderpass() {
    if (m_Renderpass) vkDestroyRenderPass(VulkanState::Ref().GetDevice(), m_Renderpass, nullptr);
  }

  void Renderpass::AddColorAttachment(VkFormat format, VkSampleCountFlagBits samples, LoadStoreOp op, VkImageLayout initial_layout, VkImageLayout final_layout, VkImageLayout subpass_layout) {
    AttachmentDescription desc = build_attachment_description(format, samples, op, initial_layout, final_layout, subpass_layout, m_AttachmentCount++);
    m_Attachments.push_back(desc.Description);
    m_ColorAttachmentsRef.push_back(desc.Reference);
  }

  void Renderpass::SetDepthAttachment(VkFormat format, VkSampleCountFlagBits samples, LoadStoreOp op, VkImageLayout initial_layout, VkImageLayout final_layout, VkImageLayout subpass_layout) {
    AttachmentDescription desc = build_attachment_description(format, samples, op, initial_layout, final_layout, subpass_layout, m_AttachmentCount++);
    m_Attachments.push_back(desc.Description);
    m_DepthAttachmentRef = desc.Reference;
    m_HasDepthAttachment = true;
  }

  void Renderpass::Build(VkPipelineBindPoint bind_point, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask, VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask) {
    ASSERT(m_Renderpass == VK_NULL_HANDLE);

    VkSubpassDescription subpass    = {};
    subpass.flags                   = 0u;
    subpass.pipelineBindPoint       = bind_point;
    subpass.inputAttachmentCount    = 0u;
    subpass.pInputAttachments       = nullptr;
    subpass.colorAttachmentCount    = static_cast<uint32_t>(m_ColorAttachmentsRef.size());
    subpass.pColorAttachments       = m_ColorAttachmentsRef.data();
    subpass.pResolveAttachments     = nullptr;
    subpass.pDepthStencilAttachment = m_HasDepthAttachment ? &m_DepthAttachmentRef : nullptr;
    subpass.preserveAttachmentCount = 0u;
    subpass.pPreserveAttachments    = nullptr;

    VkSubpassDependency dependency  = {};
    dependency.srcSubpass           = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass           = 0u;
    dependency.srcStageMask         = src_stage_mask;
    dependency.dstStageMask         = dst_stage_mask;
    dependency.srcAccessMask        = dst_access_mask;
    dependency.dstAccessMask        = dst_access_mask;
    dependency.dependencyFlags      = 0u;

    VkRenderPassCreateInfo create_info = {};
    create_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.pNext                  = nullptr;
    create_info.flags                  = 0u;
    create_info.attachmentCount        = static_cast<uint32_t>(m_Attachments.size());
    create_info.pAttachments           = m_Attachments.data();
    create_info.subpassCount           = 1u;
    create_info.pSubpasses             = &subpass;
    create_info.dependencyCount        = 1u;
    create_info.pDependencies          = &dependency;

    VK_CALL(vkCreateRenderPass(VulkanState::Ref().GetDevice(), &create_info, nullptr, &m_Renderpass));
  }

}
