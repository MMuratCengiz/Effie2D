#include "Effie/ShaderReflection.h"
#include "Effie/Utilities.h"
#include "dawn/utils/WGPUHelpers.h"
#include "tint/source.h"
#include "tint/reader/wgsl/parser.h"

using namespace Effie;

ShaderReflection::ShaderReflection(const std::initializer_list<ShaderInfo>& shaders,
	RenderContext* context,
	const ShaderOptions& options) : context(context)
{
	for (const auto& shaderInfo : shaders)
	{
		std::string contents = Effie::Utilities::ReadFile(shaderInfo.Path);

		// todo use glsl + spirv
		wgpu::ShaderModuleWGSLDescriptor wgslDesc;
		wgslDesc.source = contents.c_str();

		wgpu::ShaderModuleDescriptor descriptor;
		descriptor.nextInChain = &wgslDesc;
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

void ShaderReflection::ParseShader(const wgpu::ShaderStage& stage, tint::Program program)
{
	auto& p = program.ASTNodes();
	auto& n = program.Types();
	auto& s = program.SemNodes();

	std::vector<ShaderVarTypeInfo> types;

	for (auto astNode : program.ASTNodes().Objects())
	{
		auto s1 = astNode->source;
		auto a1 = astNode->TypeInfo();
		int we2 = 1;
	}

	for (auto semNode : program.SemNodes().Objects())
	{
		auto a2 = semNode->TypeInfo();
		int we3 = 1;
	}

	for (auto t : program.Types())
	{
		ShaderVarTypeInfo& typeInfo = types.emplace_back();
		typeInfo.Name = t->FriendlyName(program.Symbols());
		SetShaderTypeEnum(t, typeInfo);
		typeInfo.Size = t->Size();
		typeInfo.Alignment = t->Align();
	}

	int it = 0;
	program.Symbols().Foreach([&](const tint::Symbol& symbol, const std::string& symbolName)
	{
		if (symbol.value() < types.size())
		{
			auto toipe = types[symbol.value()];
			auto toipe2 = types[it];
			int l = 1;
			it++;
		}
	});

	int i = 0;
}

void ShaderReflection::SetShaderTypeEnum(const tint::sem::Type* t, ShaderVarTypeInfo& typeInfo) const
{
	if (t->is_signed_integer_scalar())
	{
		typeInfo.Type = ShaderVarType::Int32;
	}
	else if (t->is_integer_scalar())
	{
		typeInfo.Type = ShaderVarType::UInt32;
	}
	else if (t->is_bool_scalar_or_vector() && !t->is_bool_vector())
	{
		typeInfo.Type = ShaderVarType::Bool;
	}
	else if (t->is_bool_vector())
	{
		typeInfo.Type = ShaderVarType::BoolVector;
	}
	else if (t->is_float_scalar())
	{
		typeInfo.Type = ShaderVarType::Float;
	}
	else if (t->is_float_vector())
	{
		typeInfo.Type = ShaderVarType::FloatVector;
	}
	else if (t->is_float_matrix())
	{
		typeInfo.Type = ShaderVarType::FloatMatrix;
	}
	else if (t->is_abstract_or_scalar() && t->is_float_scalar())
	{
		typeInfo.Type = ShaderVarType::AbstractFloat;
	}
	else if (t->is_abstract_or_scalar() && t->is_integer_scalar())
	{
		typeInfo.Type = ShaderVarType::AbstractUInt32;
	}
}
