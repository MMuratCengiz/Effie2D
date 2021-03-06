#pragma once

#include "Effie/RenderContext.h"
#include "unordered_map"
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

struct BindGroup
{
	std::vector<wgpu::BindGroupLayoutEntry> LayoutEntries;
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

	GETTER(wgpu::VertexState&, GetVertexState, vertexState)
	GETTER(bool, HasFragmentState, hasFragmentState)
	GETTER(wgpu::FragmentState&, GetFragmentState, fragmentState)
private:
	void OnEachShader(const SPIRVInfo& shaderInfo);
	SpvDecoration GetDecoration(const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource);
	wgpu::VertexFormat SPVToWGPUType(const spirv_cross::SPIRType& type);
	void CreateVertexState(const SPIRVInfo& shaderInfo, const spirv_cross::Compiler& compiler, spirv_cross::SmallVector<spirv_cross::Resource>& stageInputs);
	void CreateFragmentState(const SPIRVInfo& shaderInfo, spirv_cross::SmallVector<spirv_cross::Resource>& imageOutputs);

	RenderContext* context;
	ShaderOptions options;

	bool hasFragmentState = false;
	wgpu::VertexState vertexState{ };
	wgpu::FragmentState fragmentState{ };

	std::unordered_map<wgpu::ShaderStage, wgpu::ShaderModule> modules;

	std::vector<SPIRVInfo> spirvInfos;
	std::vector<wgpu::VertexAttribute> vertexAttributes;
	std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries;
	wgpu::BindGroupLayout bindGroupLayout;
};

}
