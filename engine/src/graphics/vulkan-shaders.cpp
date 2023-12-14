#include "vulkan-shaders.h"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <engine/log.h>
#include <engine/types.h>
#include "vulkan-state.h"

namespace mau {

  std::string read_file(std::string_view file_path) {
    std::ifstream file(file_path.data());
    if (!file) {
      LOG_ERROR("failed to open file: %s", file_path.data());
      return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

  std::vector<TUint32> compile_shader(shaderc::Compiler& compiler, shaderc_shader_kind kind, std::string_view file_path) {
    std::string raw = read_file(file_path);
    if (raw.empty()) return std::vector<TUint32>();

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(raw, kind, file_path.data());

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
      LOG_ERROR("%s", result.GetErrorMessage().c_str());
      return std::vector<TUint32>();
    }

    return std::vector<TUint32>(result.begin(), result.end());
  }

  Shader::Shader(std::string_view shader_path, shaderc_shader_kind kind, VkShaderStageFlagBits stage): m_Stage(stage) {
    static shaderc::Compiler compiler;

    std::vector<TUint32> shader_code = compile_shader(compiler, kind, shader_path);
    if (!shader_code.size()) return;

    VkShaderModuleCreateInfo create_info = {};
    create_info.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext                    = nullptr;
    create_info.flags                    = 0u;
    create_info.codeSize                 = shader_code.size() * sizeof(shader_code[0]);
    create_info.pCode                    = shader_code.data();

    VK_CALL(vkCreateShaderModule(VulkanState::Ref().GetDevice(), &create_info, nullptr, &m_Module));
  }

  Shader::~Shader() {
    if (m_Module) vkDestroyShaderModule(VulkanState::Ref().GetDevice(), m_Module, nullptr);
  }

  VkPipelineShaderStageCreateInfo Shader::GetShaderStageInfo() const {
    VkPipelineShaderStageCreateInfo shader_stage_info = {};
    shader_stage_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_info.pNext                           = nullptr;
    shader_stage_info.flags                           = 0u;
    shader_stage_info.stage                           = m_Stage;
    shader_stage_info.module                          = m_Module;
    shader_stage_info.pName                           = "main";
    shader_stage_info.pSpecializationInfo             = nullptr;

    return shader_stage_info;
  }
  
  VertexShader::VertexShader(std::string_view shader_path): Shader(shader_path, shaderc_glsl_vertex_shader, VK_SHADER_STAGE_VERTEX_BIT) {}

  FragmentShader::FragmentShader(std::string_view shader_path): Shader(shader_path, shaderc_glsl_fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT) {}

}
