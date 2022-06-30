#include "Effie/ShaderReflection.h"
#include "Effie/Utilities.h"
#include "dawn/utils/WGPUHelpers.h"
#include "Effie/SpirvHelper.h"

using namespace Effie;

ShaderReflection::ShaderReflection(const std::initializer_list<ShaderInfo>& shaders,
	RenderContext* context,
	const ShaderOptions& options) : context(context)
{
	for (const auto& shaderInfo : shaders)
	{
		std::string contents = Effie::Utilities::ReadFile(shaderInfo.Path);

		std::vector<uint32_t> spv = std::move(SpirvHelper::GLSLtoSPV(shaderInfo.Stage, contents.c_str()));
		SPIRVInfo& info = spirvInfos.emplace_back();
		info.Stage = shaderInfo.Stage;
		info.Data = std::move(spv);

		OnEachShader(info);
	}

	wgpu::BindGroupLayoutDescriptor descriptor;
	descriptor.entryCount = static_cast<uint32_t>(bindGroupLayoutEntries.size());
	descriptor.entries = bindGroupLayoutEntries.data();

	bindGroupLayout = context->Device.CreateBindGroupLayout(&descriptor);
}

void ShaderReflection::OnEachShader(const SPIRVInfo& shaderInfo)
{
	spirv_cross::Compiler compiler(shaderInfo.Data);

	auto shaderResources = compiler.get_shader_resources();
	auto samplers = shaderResources.sampled_images;
	auto uniforms = shaderResources.uniform_buffers;
	auto shaderPushConstants = shaderResources.push_constant_buffers;
	auto imageOutputs = shaderResources.stage_outputs;
	auto stageInputs = shaderResources.stage_inputs;

	if (shaderInfo.Stage == wgpu::ShaderStage::Vertex)
	{
		CreateVertexState(shaderInfo, compiler, stageInputs);
	}
	else if (shaderInfo.Stage == wgpu::ShaderStage::Fragment)
	{
		hasFragmentState = true;
		CreateFragmentState(shaderInfo, imageOutputs);
	}

	for (const spirv_cross::Resource& resource : samplers)
	{
		auto& bindLayoutEntry = bindGroupLayoutEntries.emplace_back();

		SpvDecoration decoration = GetDecoration(compiler, resource);

		bindLayoutEntry.visibility = shaderInfo.Stage;
		bindLayoutEntry.binding = decoration.binding;
		bindLayoutEntry.sampler = { };
		bindLayoutEntry.sampler.type = wgpu::SamplerBindingType::Filtering;
	}

	for (const spirv_cross::Resource& resource : uniforms)
	{
		auto& bindLayoutEntry = bindGroupLayoutEntries.emplace_back();

		SpvDecoration decoration = GetDecoration(compiler, resource);

		bindLayoutEntry.visibility = shaderInfo.Stage;
		bindLayoutEntry.binding = decoration.binding;
		bindLayoutEntry.buffer = { };
		bindLayoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
	}

	if (!shaderPushConstants.empty())
	{
		LOG(ERROR) << "Push constants not yet supported by WebGPU";
	}
}
void ShaderReflection::CreateVertexState(const SPIRVInfo& shaderInfo,
	const spirv_cross::Compiler& compiler,
	spirv_cross::SmallVector<spirv_cross::Resource>& stageInputs)
{
	uint32_t offsetIter = 0;

	wgpu::ShaderModuleSPIRVDescriptor spirvDescriptor;
	spirvDescriptor.code = shaderInfo.Data.data();
	spirvDescriptor.codeSize = shaderInfo.Data.size();

	wgpu::ShaderModuleDescriptor descriptor;
	descriptor.nextInChain = &spirvDescriptor;
	wgpu::ShaderModule module = context->Device.CreateShaderModule(&descriptor);
	vertexState.module = std::move(module);

	std::sort(stageInputs.begin(),
		stageInputs.end(),
		[&](const spirv_cross::Resource& r1, const spirv_cross::Resource& r2)
		{
			SpvDecoration decoration1 = GetDecoration(compiler, r1);
			SpvDecoration decoration2 = GetDecoration(compiler, r2);
			return decoration1.location < decoration2.location;
		});

	std::vector<wgpu::VertexBufferLayout> vertexBufferLayout;
	for (const spirv_cross::Resource& resource : stageInputs)
	{
		SpvDecoration decoration = GetDecoration(compiler, resource);

		auto& attribute = vertexAttributes.emplace_back();
		attribute.format = SPVToWGPUType(decoration.type);
		attribute.offset = offsetIter;
		attribute.shaderLocation = decoration.location;
		offsetIter += decoration.size * decoration.type.vecsize;

		if (!options.InterleavedMode)
		{
			wgpu::VertexBufferLayout& layout = vertexBufferLayout.emplace_back();
			layout.attributeCount = 1;
			layout.arrayStride = decoration.size * decoration.type.vecsize; // maybe simply 0?
			layout.attributes = &attribute;
			layout.stepMode = wgpu::VertexStepMode::Vertex;
		}
	}

	if (options.InterleavedMode)
	{
		wgpu::VertexBufferLayout& layout = vertexBufferLayout.emplace_back();
		layout.attributeCount = vertexAttributes.size();
		layout.arrayStride = offsetIter;
		layout.attributes = vertexAttributes.data();
		layout.stepMode = wgpu::VertexStepMode::Vertex;
	}

	vertexState.bufferCount = stageInputs.size();
	vertexState.buffers = vertexBufferLayout.data();
	vertexState.entryPoint = "main";
}

void ShaderReflection::CreateFragmentState(const SPIRVInfo& shaderInfo,
	spirv_cross::SmallVector<spirv_cross::Resource>& imageOutputs)
{
	wgpu::ShaderModuleSPIRVDescriptor spirvDescriptor;
	spirvDescriptor.code = shaderInfo.Data.data();
	spirvDescriptor.codeSize = shaderInfo.Data.size();

	wgpu::ShaderModuleDescriptor descriptor;
	descriptor.nextInChain = &spirvDescriptor;
	wgpu::ShaderModule module = context->Device.CreateShaderModule(&descriptor);
	fragmentState.module = std::move(module);
	fragmentState.entryPoint = "main";
	fragmentState.targetCount = imageOutputs.size();
}

wgpu::VertexFormat ShaderReflection::SPVToWGPUType(const spirv_cross::SPIRType& type)
{
	auto make32Int = [](const uint32_t& numOfElements) -> wgpu::VertexFormat
	{
		if (numOfElements == 1) return wgpu::VertexFormat::Sint32;
		if (numOfElements == 2) return wgpu::VertexFormat::Sint32x2;
		if (numOfElements == 3) return wgpu::VertexFormat::Sint32x3;
		if (numOfElements == 4) return wgpu::VertexFormat::Sint32x4;
		return wgpu::VertexFormat::Undefined;
	};

	auto make64Int = [&](const uint32_t& numOfElements) -> wgpu::VertexFormat
	{
		LOG(ERROR) << "Shader reflection error, 64 bit ints not supported";
		return make32Int(numOfElements);
	};

	auto make32UInt = [](const uint32_t& numOfElements) -> wgpu::VertexFormat
	{
		if (numOfElements == 1) return wgpu::VertexFormat::Uint32;
		if (numOfElements == 2) return wgpu::VertexFormat::Uint32x2;
		if (numOfElements == 3) return wgpu::VertexFormat::Uint32x3;
		if (numOfElements == 4) return wgpu::VertexFormat::Uint32x4;
		return wgpu::VertexFormat::Undefined;
	};

	auto make64UInt = [&](const uint32_t& numOfElements) -> wgpu::VertexFormat
	{
		LOG(ERROR) << "Shader reflection error, 64 bit ints not supported";
		return make32UInt(numOfElements);
	};

	auto make32Float = [](const uint32_t& numOfElements) -> wgpu::VertexFormat
	{
		if (numOfElements == 1) return wgpu::VertexFormat::Float32;
		if (numOfElements == 2) return wgpu::VertexFormat::Float32x2;
		if (numOfElements == 3) return wgpu::VertexFormat::Float32x3;
		if (numOfElements == 4) return wgpu::VertexFormat::Float32x4;
		return wgpu::VertexFormat::Undefined;
	};

	auto make64Float = [&](const uint32_t& numOfElements) -> wgpu::VertexFormat
	{
		LOG(ERROR) << "Shader reflection error, 64 bit ints not supported";
		return make32Float(numOfElements);
	};

	switch (type.basetype)
	{
	default:
		break;
	case spirv_cross::SPIRType::Short:
	case spirv_cross::SPIRType::UShort:
	case spirv_cross::SPIRType::Int:
		return make32Int(type.vecsize);
	case spirv_cross::SPIRType::Int64:
		return make64Int(type.vecsize);
	case spirv_cross::SPIRType::UInt:
		return make32UInt(type.vecsize);
	case spirv_cross::SPIRType::UInt64:
		return make64UInt(type.vecsize);
	case spirv_cross::SPIRType::Float:
		return make32Float(type.vecsize);
	case spirv_cross::SPIRType::Double:
		return make64Float(type.vecsize);
	}

	LOG(ERROR) << "Unsupported type specified in vertex input";
	return wgpu::VertexFormat::Undefined;
}

SpvDecoration ShaderReflection::GetDecoration(const spirv_cross::Compiler& compiler,
	const spirv_cross::Resource& resource)
{
	SpvDecoration decoration{ };

	decoration.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
	decoration.type = compiler.get_type(resource.type_id);

	if (decoration.type.basetype == spirv_cross::SPIRType::Struct)
	{
		uint32_t structSize = compiler.get_declared_struct_size(decoration.type);
		// Doesn't seem necessary at the moment:
		// uint32_t m1_size = compiler.get_declared_struct_member_size( decoration.type, 0 );
		uint32_t offsetIter = 0;

		uint32_t size;

		for (uint32_t i = 0; offsetIter != structSize; i++)
		{
			size = compiler.get_declared_struct_member_size(decoration.type, i);

			auto& child = decoration.children.emplace_back();

			child.offset = offsetIter;
			child.size = size;
			child.name = compiler.get_member_name(resource.base_type_id, i);

			offsetIter += size;
		}

		decoration.size = structSize;
	}

	decoration.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
	decoration.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

	uint32_t totalArraySize = 0;

	for (uint32_t dimensionSize : decoration.type.array)
	{
		totalArraySize += dimensionSize;
	}

	decoration.arraySize = totalArraySize == 0 ? 1 : decoration.size / totalArraySize;
	decoration.name = resource.name;

	return decoration;
}