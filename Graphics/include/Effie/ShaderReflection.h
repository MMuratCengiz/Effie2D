#pragma once

#include "Effie/RenderContext.h"

struct ShaderInfo {
	wgpu::ShaderStage Stage;
	std::string Path;
};

namespace Effie
{

class ShaderReflection
{
public:
	ShaderReflection(const std::initializer_list<ShaderInfo>& shaders);
};

}
