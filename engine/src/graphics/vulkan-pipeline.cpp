#include "vulkan-pipeline.h"

#include "vulkan-state.h"

namespace mau {

  InputLayout::InputLayout() { }

  InputLayout::~InputLayout() { }

  void InputLayout::AddBindingDesc(TUint32 binding, TUint32 stride) {
    VkVertexInputBindingDescription desc = {};
    desc.binding = binding;
    desc.stride = stride;
    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    m_BindingDesc.push_back(desc);
  }

  void InputLayout::AddAttributeDesc(TUint32 location, TUint32 binding,
                                     VkFormat format, TUint32 offset) {
    VkVertexInputAttributeDescription desc = {};
    desc.location = location;
    desc.binding = binding;
    desc.format = format;
    desc.offset = offset;

    m_AttributeDesc.push_back(desc);
  }

  Pipeline::Pipeline(
      Handle<VertexShader>   vertex_shader,
      Handle<FragmentShader> fragment_shader, Handle<Renderpass> renderpass,
      const InputLayout &input_layout, Handle<PushConstantBase> push_constant,
      const std::vector<VkDescriptorSetLayout> &descriptor_layouts,
      const VkSampleCountFlagBits              &sample_count) {
    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vertex_shader->GetShaderStageInfo(),
        fragment_shader->GetShaderStageInfo()};

    // vertex input
    VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
    vertex_input_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state.pNext = nullptr;
    vertex_input_state.flags = 0u;
    vertex_input_state.vertexBindingDescriptionCount =
        static_cast<uint32_t>(input_layout.GetBindingDesc().size());
    vertex_input_state.pVertexBindingDescriptions =
        input_layout.GetBindingDesc().data();
    vertex_input_state.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(input_layout.GetAttributeDesc().size());
    vertex_input_state.pVertexAttributeDescriptions =
        input_layout.GetAttributeDesc().data();

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
    input_assembly_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state.pNext = nullptr;
    input_assembly_state.flags = 0u;
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state.primitiveRestartEnable = false;

    // dynamic states
    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                       VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.pNext = nullptr;
    dynamic_state.flags = 0u;
    dynamic_state.dynamicStateCount = ARRAY_SIZE(dynamic_states);
    dynamic_state.pDynamicStates = dynamic_states;

    // viewport, empty because dynamic
    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pNext = nullptr;
    viewport_state.flags = 0u;
    viewport_state.viewportCount = 1u;
    viewport_state.pViewports = nullptr;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = nullptr;

    // rasterizer
    VkPipelineRasterizationStateCreateInfo raster_state = {};
    raster_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_state.pNext = nullptr;
    raster_state.flags = 0u;
    raster_state.depthClampEnable = VK_FALSE;
    raster_state.rasterizerDiscardEnable = VK_FALSE;
    raster_state.polygonMode = VK_POLYGON_MODE_FILL;
    raster_state.cullMode = VK_CULL_MODE_BACK_BIT;
    raster_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster_state.depthBiasEnable = VK_FALSE;
    raster_state.depthBiasConstantFactor = 0.0f;
    raster_state.depthBiasClamp = 0.0f;
    raster_state.depthBiasSlopeFactor = 0.0f;
    raster_state.lineWidth = 1.0f;

    // multisampling
    VkPipelineMultisampleStateCreateInfo multisample_state = {};
    multisample_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.pNext = nullptr;
    multisample_state.flags = 0u;
    multisample_state.rasterizationSamples = sample_count;
    multisample_state.sampleShadingEnable = VK_FALSE;
    multisample_state.minSampleShading = 1.0f;
    multisample_state.pSampleMask = nullptr;
    multisample_state.alphaToCoverageEnable = VK_FALSE;
    multisample_state.alphaToOneEnable = VK_FALSE;

    // color blending
    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_state = {};
    color_blend_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.pNext = nullptr;
    color_blend_state.flags = 0u;
    color_blend_state.logicOpEnable = VK_FALSE;
    color_blend_state.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state.attachmentCount = 1u;
    color_blend_state.pAttachments = &color_blend_attachment;
    color_blend_state.blendConstants[0] = 0.0f;
    color_blend_state.blendConstants[1] = 0.0f;
    color_blend_state.blendConstants[2] = 0.0f;
    color_blend_state.blendConstants[3] = 0.0f;

    // depth stencil
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
    depth_stencil_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state.pNext = nullptr;
    depth_stencil_state.flags = 0u;
    depth_stencil_state.depthTestEnable = VK_TRUE;
    depth_stencil_state.depthWriteEnable = VK_TRUE;
    depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state.stencilTestEnable = VK_FALSE;

    VkPushConstantRange push_constant_range = {};
    if (push_constant) {
      push_constant_range = push_constant->GetRange();
    }

    // create pipeline layout
    VkPipelineLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.pNext = nullptr;
    layout_create_info.flags = 0u;
    layout_create_info.setLayoutCount =
        static_cast<uint32_t>(descriptor_layouts.size());
    layout_create_info.pSetLayouts = descriptor_layouts.data();
    layout_create_info.pushConstantRangeCount = push_constant ? 1 : 0;
    layout_create_info.pPushConstantRanges =
        push_constant ? &push_constant_range : nullptr;

    VK_CALL(vkCreatePipelineLayout(VulkanState::Ref().GetDevice(),
                                   &layout_create_info, nullptr,
                                   &m_PipelineLayout));

    VkGraphicsPipelineCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0u;
    create_info.stageCount = ARRAY_SIZE(shader_stages);
    create_info.pStages = shader_stages;
    create_info.pVertexInputState = &vertex_input_state;
    create_info.pInputAssemblyState = &input_assembly_state;
    create_info.pTessellationState = nullptr;
    create_info.pViewportState = &viewport_state;
    create_info.pRasterizationState = &raster_state;
    create_info.pMultisampleState = &multisample_state;
    create_info.pDepthStencilState = &depth_stencil_state;
    create_info.pColorBlendState = &color_blend_state;
    create_info.pDynamicState = &dynamic_state;
    create_info.layout = m_PipelineLayout;
    create_info.renderPass = renderpass->Get();
    create_info.subpass = 0;
    create_info.basePipelineHandle = VK_NULL_HANDLE;
    create_info.basePipelineIndex = -1;

    VK_CALL(vkCreateGraphicsPipelines(VulkanState::Ref().GetDevice(),
                                      VK_NULL_HANDLE, 1, &create_info, nullptr,
                                      &m_Pipeline));
  }

  Pipeline::~Pipeline() {
    if (m_PipelineLayout)
      vkDestroyPipelineLayout(VulkanState::Ref().GetDevice(), m_PipelineLayout,
                              nullptr);
    if (m_Pipeline)
      vkDestroyPipeline(VulkanState::Ref().GetDevice(), m_Pipeline, nullptr);
  }

  bool validate_create_info(const RTPipelineCreateInfo &create_info) {
    return create_info.ClosestHit && create_info.Miss && create_info.RayGen;
  }

  RTPipeline::RTPipeline(const RTPipelineCreateInfo &create_info) {
    ASSERT(validate_create_info(create_info));

    VkPipelineShaderStageCreateInfo shader_stages[3] = {
        create_info.ClosestHit->GetShaderStageInfo(),
        create_info.RayGen->GetShaderStageInfo(),
        create_info.Miss->GetShaderStageInfo(),
    };

    VkRayTracingShaderGroupCreateInfoKHR base_shader_group_info = {
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
        .pNext = nullptr,
        .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
        .generalShader = VK_SHADER_UNUSED_KHR,
        .closestHitShader = VK_SHADER_UNUSED_KHR,
        .anyHitShader = VK_SHADER_UNUSED_KHR,
        .intersectionShader = VK_SHADER_UNUSED_KHR,
        .pShaderGroupCaptureReplayHandle = nullptr,
    };
    VkRayTracingShaderGroupCreateInfoKHR shader_groups[3] = {
        base_shader_group_info, base_shader_group_info, base_shader_group_info};

    shader_groups[0].type =
        VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    shader_groups[0].closestHitShader = 0u;
    shader_groups[1].generalShader = 1u;
    shader_groups[2].generalShader = 2u;

    VkPushConstantRange push_constant_range = {};
    if (create_info.PushConstant) {
      push_constant_range = create_info.PushConstant->GetRange();
    }

    VkPipelineLayoutCreateInfo layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .setLayoutCount =
            static_cast<uint32_t>(create_info.DescriptorLayouts.size()),
        .pSetLayouts = create_info.DescriptorLayouts.data(),
        .pushConstantRangeCount = create_info.PushConstant ? 1u : 0u,
        .pPushConstantRanges =
            create_info.PushConstant ? &push_constant_range : nullptr,
    };

    VK_CALL(vkCreatePipelineLayout(VulkanState::Ref().GetDevice(),
                                   &layout_create_info, nullptr,
                                   &m_PipelineLayout));

    VkRayTracingPipelineCreateInfoKHR pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0u,
        .stageCount = ARRAY_SIZE(shader_stages),
        .pStages = shader_stages,
        .groupCount = ARRAY_SIZE(shader_groups),
        .pGroups = shader_groups,
        .maxPipelineRayRecursionDepth = 6u,
        .pLibraryInfo = nullptr,
        .pLibraryInterface = nullptr,
        .pDynamicState = nullptr,
        .layout = m_PipelineLayout,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0u,
    };

    VK_CALL(vkCreateRayTracingPipelinesKHR(
        VulkanState::Ref().GetDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
        &pipeline_create_info, nullptr, &m_Pipeline));

    CreateShaderBindingTable();
  }

  RTPipeline::~RTPipeline() {
    if (m_PipelineLayout)
      vkDestroyPipelineLayout(VulkanState::Ref().GetDevice(), m_PipelineLayout,
                              nullptr);
    if (m_Pipeline)
      vkDestroyPipeline(VulkanState::Ref().GetDevice(), m_Pipeline, nullptr);
  }

  RTSBTRegion RTPipeline::GetSBTRegion() const {
    RTSBTRegion region = {
        .RayGen = m_RayGenRegion,
        .RayMiss = m_RayMissRegion,
        .RayClosestHit = m_RayClosestHitRegion,
        .RayCall = m_RayCallRegion,
    };

    return region;
  }

  void RTPipeline::CreateShaderBindingTable() {
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_properties =
        VulkanState::Ref().GetRTPipelineProperties();

    TUint32 miss_count = 1u;
    TUint32 chit_count = 1u;
    TUint32 handle_count = 1u + miss_count + chit_count;
    TUint64 handle_size =
        static_cast<TUint64>(rt_pipeline_properties.shaderGroupHandleSize);
    TUint64 handle_size_aligned = align_up(
        handle_size, rt_pipeline_properties.shaderGroupHandleAlignment);

    m_RayGenRegion = {
        .deviceAddress = 0u,
        .stride = align_up(handle_size_aligned,
                           rt_pipeline_properties.shaderGroupBaseAlignment),
        .size = align_up(handle_size_aligned,
                         rt_pipeline_properties.shaderGroupBaseAlignment),
    };

    m_RayMissRegion = {
        .deviceAddress = 0u,
        .stride = handle_size_aligned,
        .size = align_up(miss_count * handle_size_aligned,
                         rt_pipeline_properties.shaderGroupBaseAlignment),
    };

    m_RayClosestHitRegion = {
        .deviceAddress = 0u,
        .stride = handle_size_aligned,
        .size = align_up(chit_count * handle_size_aligned,
                         rt_pipeline_properties.shaderGroupBaseAlignment),
    };

    m_RayCallRegion = {
        .deviceAddress = 0u,
        .stride = 0u,
        .size = 0u,
    };

    TUint64        data_size = static_cast<TUint64>(handle_count * handle_size);
    Vector<TUint8> handles(data_size);
    VK_CALL(vkGetRayTracingShaderGroupHandlesKHR(VulkanState::Ref().GetDevice(),
                                                 m_Pipeline, 0, handle_count,
                                                 data_size, handles.data()));

    VkDeviceSize sbt_size = m_RayGenRegion.size + m_RayMissRegion.size +
                            m_RayClosestHitRegion.size + m_RayCallRegion.size;
    m_SBTBuffer = make_handle<Buffer>(
        sbt_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    VkDeviceAddress sbt_address = m_SBTBuffer->GetDeviceAddress();
    m_RayClosestHitRegion.deviceAddress = sbt_address;
    m_RayGenRegion.deviceAddress = sbt_address + m_RayClosestHitRegion.size;
    m_RayMissRegion.deviceAddress =
        sbt_address + m_RayClosestHitRegion.size + m_RayGenRegion.size;

    auto get_handle = [&](TUint32 i) {
      return handles.data() + i * handle_size;
    };

    TUint8 *sbt_buffer_data = reinterpret_cast<TUint8 *>(m_SBTBuffer->Map());
    TUint8 *data = nullptr;
    TUint32 handle_index = 0u;

    // closest hit
    data = sbt_buffer_data;
    for (TUint32 c = 0; c < chit_count; c++) {
      memcpy(data, get_handle(handle_index++), handle_size);
      data += m_RayClosestHitRegion.size;
    }

    // ray gen
    data = sbt_buffer_data + m_RayClosestHitRegion.size;
    memcpy(data, get_handle(handle_index++), handle_size);

    // ray miss
    data = sbt_buffer_data + m_RayClosestHitRegion.size + m_RayGenRegion.size;
    for (TUint32 c = 0; c < miss_count; c++) {
      memcpy(data, get_handle(handle_index++), handle_size);
      data += m_RayMissRegion.size;
    }

    m_SBTBuffer->UnMap();
  }

} // namespace mau
