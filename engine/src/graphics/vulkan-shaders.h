#pragma once

#include <string_view>
#include <engine/enums.h>
#include <shaderc/shaderc.hpp>
#include "common.h"

namespace mau {

  class Shader: public HandledObject {
  public:
    Shader(std::string_view shader_path, shaderc_shader_kind kind, VkShaderStageFlagBits stage);
    virtual ~Shader();
  public:
    VkPipelineShaderStageCreateInfo GetShaderStageInfo() const;
  private:
    VkShaderModule        m_Module = VK_NULL_HANDLE;
    VkShaderStageFlagBits m_Stage  = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
  };

  class VertexShader: public Shader {
  public:
    VertexShader(std::string_view shader_path);
    ~VertexShader() = default;
  };

  class FragmentShader : public Shader {
  public:
    FragmentShader(std::string_view shader_path);
    ~FragmentShader() = default;
  };

}
