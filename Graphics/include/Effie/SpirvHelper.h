#pragma once

#include "Effie/CommonIncludes.h"
#include <glslang/SPIRV/GlslangToSpv.h>
#include "dawn/webgpu_cpp.h"
namespace Effie
{

class SpirvHelper
{
public:
	static void Init();
	static void Destroy();
	static void InitResources(TBuiltInResource& Resources);
	static EShLanguage GetLanguage(const wgpu::ShaderStage & stage);
	static std::vector<uint32_t> GLSLtoSPV(const wgpu::ShaderStage& stage, const char* pShader);
};

}