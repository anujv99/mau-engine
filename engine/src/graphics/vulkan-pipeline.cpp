#include "vulkan-pipeline.h"

#include "vulkan-state.h"

namespace mau {

  Pipeline::Pipeline(Handle<VertexShader> vertex_shader, Handle<FragmentShader> fragment_shader, Handle<Renderpass> renderpass, const InputLayout& input_layout, Handle<PushConstantBase> push_constant, const std::vector<VkDescriptorSetLayout>& descriptor_layouts) {
    VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_shader->GetShaderStageInfo(), fragment_shader->GetShaderStageInfo() };

    // vertex input
    VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
    vertex_input_state.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state.pNext                                = nullptr;
    vertex_input_state.flags                                = 0u;
    vertex_input_state.vertexBindingDescriptionCount        = static_cast<uint32_t>(input_layout.GetBindingDesc().size());
    vertex_input_state.pVertexBindingDescriptions           = input_layout.GetBindingDesc().data();
    vertex_input_state.vertexAttributeDescriptionCount      = static_cast<uint32_t>(input_layout.GetAttributeDesc().size());
    vertex_input_state.pVertexAttributeDescriptions         = input_layout.GetAttributeDesc().data();

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
    input_assembly_state.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state.pNext                                  = nullptr;
    input_assembly_state.flags                                  = 0u;
    input_assembly_state.topology                               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state.primitiveRestartEnable                 = false;

    // dynamic states
    VkDynamicState dynamic_states[]                = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType                            = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.pNext                            = nullptr;
    dynamic_state.flags                            = 0u;
    dynamic_state.dynamicStateCount                = ARRAY_SIZE(dynamic_states);
    dynamic_state.pDynamicStates                   = dynamic_states;

    // viewport, empty because dynamic
    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pNext                             = nullptr;
    viewport_state.flags                             = 0u;
    viewport_state.viewportCount                     = 1u;
    viewport_state.pViewports                        = nullptr;
    viewport_state.scissorCount                      = 1;
    viewport_state.pScissors                         = nullptr;

    // rasterizer
    VkPipelineRasterizationStateCreateInfo raster_state = {};
    raster_state.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_state.pNext                                  = nullptr;
    raster_state.flags                                  = 0u;
    raster_state.depthClampEnable                       = VK_FALSE;
    raster_state.rasterizerDiscardEnable                = VK_FALSE;
    raster_state.polygonMode                            = VK_POLYGON_MODE_FILL;
    raster_state.cullMode                               = VK_CULL_MODE_BACK_BIT;
    raster_state.frontFace                              = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster_state.depthBiasEnable                        = VK_FALSE;
    raster_state.depthBiasConstantFactor                = 0.0f;
    raster_state.depthBiasClamp                         = 0.0f;
    raster_state.depthBiasSlopeFactor                   = 0.0f;
    raster_state.lineWidth                              = 1.0f;

    // multisampling
    VkPipelineMultisampleStateCreateInfo multisample_state = {};
    multisample_state.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.pNext                                = nullptr;
    multisample_state.flags                                = 0u;
    multisample_state.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;
    multisample_state.sampleShadingEnable                  = VK_FALSE;
    multisample_state.minSampleShading                     = 1.0f;
    multisample_state.pSampleMask                          = nullptr;
    multisample_state.alphaToCoverageEnable                = VK_FALSE;
    multisample_state.alphaToOneEnable                     = VK_FALSE;

    // color blending
    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.blendEnable                         = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor                 = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.dstColorBlendFactor                 = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp                        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor                 = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.dstAlphaBlendFactor                 = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp                        = VK_BLEND_OP_ADD;
    color_blend_attachment.colorWriteMask                      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_state = {};
    color_blend_state.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.pNext                               = nullptr;
    color_blend_state.flags                               = 0u;
    color_blend_state.logicOpEnable                       = VK_FALSE;
    color_blend_state.logicOp                             = VK_LOGIC_OP_COPY;
    color_blend_state.attachmentCount                     = 1u;
    color_blend_state.pAttachments                        = &color_blend_attachment;
    color_blend_state.blendConstants[0]                   = 0.0f;
    color_blend_state.blendConstants[1]                   = 0.0f;
    color_blend_state.blendConstants[2]                   = 0.0f;
    color_blend_state.blendConstants[3]                   = 0.0f;

    // depth stencil
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
    depth_stencil_state.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state.pNext                                 = nullptr;
    depth_stencil_state.flags                                 = 0u;
    depth_stencil_state.depthTestEnable                       = VK_TRUE;
    depth_stencil_state.depthWriteEnable                      = VK_TRUE;
    depth_stencil_state.depthCompareOp                        = VK_COMPARE_OP_LESS;
    depth_stencil_state.depthBoundsTestEnable                 = VK_FALSE;
    depth_stencil_state.stencilTestEnable                     = VK_FALSE;

    VkPushConstantRange push_constant_range = {};
    if (push_constant) {
      push_constant_range = push_constant->GetRange();
    }

    // create pipeline layout
    VkPipelineLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.pNext                      = nullptr;
    layout_create_info.flags                      = 0u;
    layout_create_info.setLayoutCount             = static_cast<uint32_t>(descriptor_layouts.size());
    layout_create_info.pSetLayouts                = descriptor_layouts.data();
    layout_create_info.pushConstantRangeCount     = push_constant ? 1 : 0;
    layout_create_info.pPushConstantRanges        = push_constant ? &push_constant_range : nullptr;

    VK_CALL(vkCreatePipelineLayout(VulkanState::Ref().GetDevice(), &layout_create_info, nullptr, &m_PipelineLayout));

    VkGraphicsPipelineCreateInfo create_info = {};
    create_info.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.pNext                        = nullptr;
    create_info.flags                        = 0u;
    create_info.stageCount                   = ARRAY_SIZE(shader_stages);
    create_info.pStages                      = shader_stages;
    create_info.pVertexInputState            = &vertex_input_state;
    create_info.pInputAssemblyState          = &input_assembly_state;
    create_info.pTessellationState           = nullptr;
    create_info.pViewportState               = &viewport_state;
    create_info.pRasterizationState          = &raster_state;
    create_info.pMultisampleState            = &multisample_state;
    create_info.pDepthStencilState           = &depth_stencil_state;
    create_info.pColorBlendState             = &color_blend_state;
    create_info.pDynamicState                = &dynamic_state;
    create_info.layout                       = m_PipelineLayout;
    create_info.renderPass                   = renderpass->Get();
    create_info.subpass                      = 0;
    create_info.basePipelineHandle           = VK_NULL_HANDLE;
    create_info.basePipelineIndex            = -1;

    VK_CALL(vkCreateGraphicsPipelines(VulkanState::Ref().GetDevice(), VK_NULL_HANDLE, 1, &create_info, nullptr, &m_Pipeline));
  }

  Pipeline::~Pipeline() {
    if (m_PipelineLayout) vkDestroyPipelineLayout(VulkanState::Ref().GetDevice(), m_PipelineLayout, nullptr);
    if (m_Pipeline) vkDestroyPipeline(VulkanState::Ref().GetDevice(), m_Pipeline, nullptr);
  }

  InputLayout::InputLayout() { }

  InputLayout::~InputLayout() { }

  void InputLayout::AddBindingDesc(TUint32 binding, TUint32 stride) {
    VkVertexInputBindingDescription desc = {};
    desc.binding                         = binding;
    desc.stride                          = stride;
    desc.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;

    m_BindingDesc.push_back(desc);
  }

  void InputLayout::AddAttributeDesc(TUint32 location, TUint32 binding, VkFormat format, TUint32 offset) {
    VkVertexInputAttributeDescription desc = {};
    desc.location                          = location;
    desc.binding                           = binding;
    desc.format                            = format;
    desc.offset                            = offset;

    m_AttributeDesc.push_back(desc);
  }

}
