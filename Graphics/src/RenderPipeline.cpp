#include "Effie/RenderPipeline.h"

using namespace Effie;

RenderPipeline::RenderPipeline(RenderPipelineOptions opt)
{
	this->options = std::move(opt);

	wgpu::RenderPipelineDescriptor descriptor{ };

	ShaderReflection* shaderReflection = options.ShaderReflection;

	descriptor.vertex = shaderReflection->GetVertexState();
	descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	descriptor.primitive.cullMode = options.CullMode;
	descriptor.primitive.frontFace = options.FrontFace;
	descriptor.multisample = options.MultisampleState;
	if (options.DepthStencilState.format != wgpu::TextureFormat::Undefined)
	{
		descriptor.depthStencil = &options.DepthStencilState;
	}

	if (shaderReflection->HasFragmentState())
	{
		fragmentState = shaderReflection->GetFragmentState();
		fragmentState.targetCount = options.Targets.size();
		fragmentState.targets = options.Targets.data();
		descriptor.fragment = &fragmentState;
	}

	renderPipeline = options.Context->Device.CreateRenderPipeline(&descriptor);
}