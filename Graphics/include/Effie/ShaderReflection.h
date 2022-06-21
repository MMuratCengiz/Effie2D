#pragma once

#include "Effie/RenderContext.h"
#include "unordered_map"
#include "tint/program.h"

struct ShaderInfo
{
	wgpu::ShaderStage Stage;
	std::string Path;
};

namespace Effie
{

struct ShaderOptions
{
	bool InterleavedMode = true;
};

class ShaderReflection
{
public:
	ShaderReflection(const std::initializer_list<ShaderInfo>& shaders,
		RenderContext* context,
		const ShaderOptions& options = { });
private:
	void ParseShader(const wgpu::ShaderStage& stage, tint::Program program);
	RenderContext* context;
	std::unordered_map<wgpu::ShaderStage, wgpu::ShaderModule> modules;
	std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries;
	wgpu::BindGroupLayout bindGroupLayout;
};

}
