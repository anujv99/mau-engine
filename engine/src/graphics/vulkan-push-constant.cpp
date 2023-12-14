#include "vulkan-push-constant.h"

#include <engine/assert.h>
#include <engine/memory.h>
#include <engine/log.h>

#include "vulkan-pipeline.h"
#include "vulkan-commands.h"

namespace mau {

  PushConstantBase::PushConstantBase(TUint64 size): m_Size(size) {
    if (m_Size == 0) {
      LOG_ERROR("cannot use push constant with size 0");
      return;
    }

    MAU_ALLOC_BYTES(m_Data, m_Size);
  }

  PushConstantBase::~PushConstantBase() {
    if (m_Data) {
      MAU_FREE_BYTES(m_Data);
    }
  }

  VkPushConstantRange PushConstantBase::GetRange() const {
    VkPushConstantRange range = {};
    range.stageFlags          = VK_SHADER_STAGE_VERTEX_BIT;
    range.offset              = 0u;
    range.size                = static_cast<uint32_t>(m_Size);

    return range;
  }

  void PushConstantBase::Bind(Handle<CommandBuffer> cmd, Handle<Pipeline> pipeline) const {
    vkCmdPushConstants(cmd->Get(), pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0u, m_Size, m_Data);
  }

  void PushConstantBase::SetData(const void* const data, TUint64 size) {
    if (m_Size == 0 || m_Data == nullptr || data == nullptr) {
      LOG_ERROR("cannot set push constant data, 0 size or invalid data");
      return;
    }
    memcpy(m_Data, data, std::min(m_Size, size));
  }

  void PushConstantBase::SetData(const void* const data) {
    SetData(data, m_Size);
  }

}
