#pragma once

#include "Effie/RenderContext.h"
#include "unordered_map"
#include "tint/program.h"
#include "spirv_glsl.hpp"

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

struct SPIRVInfo
{
	wgpu::ShaderStage Stage;
	std::vector<uint32_t> Data;
};

struct StructChild
{
	std::string name;
	uint32_t offset;
	uint32_t size;
};

struct SpvDecoration
{
	spirv_cross::SPIRType type;
	uint32_t set;
	uint32_t location;
	uint32_t binding;
	uint32_t arraySize;
	uint32_t size;
	std::string name;
	std::vector<StructChild> children;
};

struct WGPUType
{

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
	void OnEachShader(const SPIRVInfo& shaderInfo);
	SpvDecoration GetDecoration(const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource);

	std::vector<SPIRVInfo> spirvInfos;
	RenderContext* context;
	ShaderOptions options;

	std::unordered_map<wgpu::ShaderStage, wgpu::ShaderModule> modules;
	std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries;
	wgpu::BindGroupLayout bindGroupLayout;
};

}
