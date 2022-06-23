#include "Effie/ShaderReflection.h"
#include "Effie/Utilities.h"
#include "dawn/utils/WGPUHelpers.h"
#include "tint/source.h"
#include "tint/reader/wgsl/parser.h"
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

		wgpu::ShaderModuleSPIRVDescriptor spirvDescriptor;
		spirvDescriptor.code = info.Data.data();
		spirvDescriptor.codeSize = info.Data.size();

		wgpu::ShaderModuleDescriptor descriptor;
		descriptor.nextInChain = &spirvDescriptor;
		wgpu::ShaderModule module = context->Device.CreateShaderModule(&descriptor);
		modules[shaderInfo.Stage] = std::move(module);

		tint::Source::File tintFile(shaderInfo.Path, contents);

		auto tintProgram = tint::reader::wgsl::Parse(&tintFile);

		ParseShader(shaderInfo.Stage, std::move(tintProgram));
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

	auto stageInputs = shaderResources.stage_inputs;
	auto samplers = shaderResources.sampled_images;
	auto uniforms = shaderResources.uniform_buffers;
	auto shaderPushConstants = shaderResources.push_constant_buffers;

	uint32_t offsetIter = 0;

	if (shaderInfo.Stage == wgpu::ShaderStage::Vertex)
	{
		std::sort(stageInputs.begin(),
			stageInputs.end(),
			[&](const spirv_cross::Resource& r1, const spirv_cross::Resource& r2)
			{
				SpvDecoration decoration1 = GetDecoration(compiler, r1);
				SpvDecoration decoration2 = GetDecoration(compiler, r2);
				return decoration1.location < decoration2.location;
			});

		for (const spirv_cross::Resource& resource : stageInputs)
		{
			SpvDecoration decoration = GetDecoration(compiler, resource);
			ShaderReflection::GLSLType gType = spvToGLSLType(decoration.type);
			createVertexInput(offsetIter, gType, decoration.location);
			offsetIter += gType.size;
		}

		if (interleavedMode)
		{
			vk::VertexInputBindingDescription
				& bindingDesc = inputBindingDescriptions.emplace_back(vk::VertexInputBindingDescription{ });
			bindingDesc.binding = 0;
			bindingDesc.inputRate = vk::VertexInputRate::eVertex; // TODO investigate later for instanced rendering
			bindingDesc.stride = offsetIter;
		}
	}

	for (const spirv_cross::Resource& resource : samplers)
	{
		DescriptorBindingCreateInfo createInfo;
		createInfo.binding = 0;
		createInfo.resource = resource;
		createInfo.stage = shaderInfo.type;
		createInfo.type = vk::DescriptorType::eCombinedImageSampler;

		createDescriptorSetBinding(compiler, createInfo);
	}

	for (const spirv_cross::Resource& resource : uniforms)
	{
		DescriptorBindingCreateInfo createInfo;
		createInfo.binding = 0;
		createInfo.resource = resource;
		createInfo.stage = shaderInfo.type;
		createInfo.type = vk::DescriptorType::eUniformBuffer;

		createDescriptorSetBinding(compiler, createInfo);
	}

	for (const spirv_cross::Resource& resource : shaderPushConstants)
	{
		SpvDecoration decoration = GetDecoration(compiler, resource);

		vk::PushConstantRange pushConstant{ };
		pushConstant.offset = 0; // TODO Could a push constant have any other offset?
		pushConstant.size = decoration.size;
		pushConstant.stageFlags = shaderInfo.type;

		pushConstants.push_back(std::move(pushConstant));

		auto& detail = pushConstantDetails.emplace_back();
		detail.stage = shaderInfo.type;
		detail.size = decoration.size;
		detail.name = decoration.name;
		detail.children = std::move(decoration.children);
	}
}

void ShaderReflection::ensureSetExists(uint32_t set)
{
	if (descriptorSetMap.find(set) == descriptorSetMap.end())
	{
		descriptorSetMap[set] = DescriptorSet{ };
		descriptorSetMap[set].id = set;
	}
}

void ShaderReflection::createVertexInput(const uint32_t& offset, const GLSLType& type, const uint32_t& location)
{
	vk::VertexInputAttributeDescription
		& desc = vertexAttributeDescriptions.emplace_back(vk::VertexInputAttributeDescription{ });

	if (interleavedMode)
	{
		desc.binding = 0;
	}
	else
	{
		vk::VertexInputBindingDescription
			& bindingDesc = inputBindingDescriptions.emplace_back(vk::VertexInputBindingDescription{ });
		bindingDesc.binding = inputBindingDescriptions.size() - 1;
		bindingDesc.inputRate = vk::VertexInputRate::eVertex; // TODO investigate later for instanced rendering
		bindingDesc.stride = 0;

		desc.binding = bindingDesc.binding;
	}

	desc.location = location;
	desc.format = type.format;
	desc.offset = offset;
}

void ShaderReflection::createDescriptorSetBinding(const spirv_cross::Compiler& compiler,
	const DescriptorBindingCreateInfo& bindingCreateInfo)
{
	SpvDecoration decoration = getDecoration(compiler, bindingCreateInfo.resource);
	auto stages = compiler.get_entry_points_and_stages();

	DescriptorSet& descriptorSet = descriptorSetMap[decoration.set];

	if (descriptorSet.descriptorSetBindingMap.find(decoration.name) != descriptorSet.descriptorSetBindingMap.end())
	{
		updateDecoration(bindingCreateInfo, decoration, descriptorSet);

		return;
	}

	vk::DescriptorSetLayoutBinding
		& layoutBinding = descriptorSet.descriptorSetLayoutBindings.emplace_back(vk::DescriptorSetLayoutBinding{ });

	layoutBinding.binding = decoration.binding;
	layoutBinding.descriptorType = bindingCreateInfo.type;
	layoutBinding.descriptorCount = decoration.arraySize;
	layoutBinding.stageFlags = bindingCreateInfo.stage;

	DescriptorSetBinding& binding = descriptorSet.descriptorSetBindings.emplace_back(DescriptorSetBinding{ });
	binding.index = descriptorSet.descriptorSetBindings.size() - 1;
	binding.size = decoration.size;
	binding.type = bindingCreateInfo.type;
	binding.name = decoration.name;
	binding.layout = layoutBinding;

	descriptorSet.descriptorSetBindingMap[decoration.name] = binding;
	descriptorSets.emplace_back(descriptorSet);
}

void ShaderReflection::updateDecoration(const ShaderReflection::DescriptorBindingCreateInfo& bindingCreateInfo,
	const ShaderReflection::SpvDecoration& decoration,
	const DescriptorSet& descriptorSet)
{
	DescriptorSetBinding& binding = descriptorSetMap[decoration.set].descriptorSetBindingMap[decoration.name];
	binding.layout.stageFlags |= bindingCreateInfo.stage;

	for (auto& set : descriptorSets)
	{
		for (auto& setBinding : set.descriptorSetBindings)
		{
			if (setBinding.name == binding.name)
			{
				setBinding.layout.stageFlags |= bindingCreateInfo.stage;

				for (auto& layoutBinding : set.descriptorSetLayoutBindings)
				{
					if (layoutBinding.binding == decoration.binding)
					{
						layoutBinding.stageFlags |= bindingCreateInfo.stage;
					}
				}
			}
		}
	}

	for (auto& setBinding : descriptorSetMap[decoration.set].descriptorSetBindings)
	{
		if (setBinding.name == binding.name)
		{
			setBinding.layout.stageFlags |= bindingCreateInfo.stage;
		}
	}

	for (auto& layoutBinding : descriptorSetMap[decoration.set].descriptorSetLayoutBindings)
	{
		if (layoutBinding.binding == decoration.binding)
		{
			layoutBinding.stageFlags |= bindingCreateInfo.stage;
		}
	}
}

ShaderReflection::GLSLType ShaderReflection::spvToGLSLType(const spirv_cross::SPIRType& type)
{
	vk::Format format = vk::Format::eUndefined;
	uint32_t size = 0;

	auto make32Int = [](const uint32_t& numOfElements) -> vk::Format
	{
		if (numOfElements == 1) return vk::Format::eR32Sint;
		if (numOfElements == 2) return vk::Format::eR32G32Sint;
		if (numOfElements == 3) return vk::Format::eR32G32B32Sint;
		if (numOfElements == 4) return vk::Format::eR32G32B32A32Sint;
		return vk::Format::eUndefined;
	};

	auto make64Int = [](const uint32_t& numOfElements) -> vk::Format
	{
		if (numOfElements == 1) return vk::Format::eR64Sint;
		if (numOfElements == 2) return vk::Format::eR64G64Sint;
		if (numOfElements == 3) return vk::Format::eR64G64B64Sint;
		if (numOfElements == 4) return vk::Format::eR64G64B64A64Sint;
		return vk::Format::eUndefined;
	};

	auto make32UInt = [](const uint32_t& numOfElements) -> vk::Format
	{
		if (numOfElements == 1) return vk::Format::eR32Uint;
		if (numOfElements == 2) return vk::Format::eR32G32Uint;
		if (numOfElements == 3) return vk::Format::eR32G32B32Uint;
		if (numOfElements == 4) return vk::Format::eR32G32B32A32Uint;
		return vk::Format::eUndefined;
	};

	auto make64UInt = [](const uint32_t& numOfElements) -> vk::Format
	{
		if (numOfElements == 1) return vk::Format::eR64Uint;
		if (numOfElements == 2) return vk::Format::eR64G64Uint;
		if (numOfElements == 3) return vk::Format::eR64G64B64Uint;
		if (numOfElements == 4) return vk::Format::eR64G64B64A64Uint;
		return vk::Format::eUndefined;
	};

	auto make32Float = [](const uint32_t& numOfElements) -> vk::Format
	{
		if (numOfElements == 1) return vk::Format::eR32Sfloat;
		if (numOfElements == 2) return vk::Format::eR32G32Sfloat;
		if (numOfElements == 3) return vk::Format::eR32G32B32Sfloat;
		if (numOfElements == 4) return vk::Format::eR32G32B32A32Sfloat;
		return vk::Format::eUndefined;
	};

	auto make64Float = [](const uint32_t& numOfElements) -> vk::Format
	{
		if (numOfElements == 1) return vk::Format::eR64Sfloat;
		if (numOfElements == 2) return vk::Format::eR64G64Sfloat;
		if (numOfElements == 3) return vk::Format::eR64G64B64Sfloat;
		if (numOfElements == 4) return vk::Format::eR64G64B64A64Sfloat;
		return vk::Format::eUndefined;
	};

	switch (type.basetype)
	{
	default:
		break;
	case spirv_cross::SPIRType::Short:
	case spirv_cross::SPIRType::UShort:
	case spirv_cross::SPIRType::Int:

		format = make32Int(type.vecsize);
		size = sizeof(int32_t);
		break;
	case spirv_cross::SPIRType::Int64:
		format = make32Int(type.vecsize);
		size = sizeof(int64_t);
		break;
	case spirv_cross::SPIRType::UInt:
		format = make32UInt(type.vecsize);
		size = sizeof(uint32_t);
		break;
	case spirv_cross::SPIRType::UInt64:
		format = make64UInt(type.vecsize);
		size = sizeof(uint64_t);
		break;
	case spirv_cross::SPIRType::Float:
		format = make32Float(type.vecsize);
		size = sizeof(float);
		break;
	case spirv_cross::SPIRType::Double:
		format = make64Float(type.vecsize);
		size = sizeof(double);
		break;
	}

	return GLSLType{ format, size * type.vecsize };
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

		uint32_t size = 0;

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

	ensureSetExists(decoration.set);

	return decoration;
}