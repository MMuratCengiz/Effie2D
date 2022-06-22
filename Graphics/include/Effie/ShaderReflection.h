#pragma once

#include "Effie/RenderContext.h"
#include "unordered_map"
#include "tint/program.h"

struct ShaderInfo
{
	wgpu::ShaderStage Stage;
	std::string Path;
};

enum class ShaderVarType
{
	AbstractFloat,
	Float,
	FloatMatrix,
	FloatVector,
	AbstractInt32,
	Int32,
	AbstractUInt32,
	UInt32,
	Int32Vector,
	Bool,
	BoolVector,
};

struct ShaderVarTypeInfo
{
	std::string Name;
	ShaderVarType Type;
	uint32_t Size;
	uint32_t Alignment;
};

struct ShaderVar
{
	std::string Name;
	std::string TypeName;
	ShaderVarType Type;
	uint32_t Size;
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
	void SetShaderTypeEnum(const tint::sem::Type* t, ShaderVarTypeInfo& typeInfo) const;

	RenderContext* context;
	std::unordered_map<wgpu::ShaderStage, wgpu::ShaderModule> modules;
	std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries;
	wgpu::BindGroupLayout bindGroupLayout;
};

}
