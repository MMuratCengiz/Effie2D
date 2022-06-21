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

	for (auto t: program.Types()) {
		auto name = t->FriendlyName(program.Symbols());
		int j = 1;
	}

	program.Symbols().Foreach([&](const tint::Symbol& symbol, const std::string& symbolName){
		int l = 1;
	});

	int i = 0;
}