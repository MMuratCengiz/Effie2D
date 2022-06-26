#include "Effie/RenderPipeline.h"

using namespace Effie;

RenderPipeline::RenderPipeline(const RenderPipelineOptions& options)
{
	wgpu::RenderPipelineDescriptor descriptor{ };

	ShaderReflection* shaderReflection = options.ShaderReflection;

	if (shaderReflection->HasFragmentState())
	{
		descriptor.fragment = &shaderReflection->GetFragmentState();
	}
	descriptor.vertex = shaderReflection->GetVertexState();
	descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	descriptor.primitive.cullMode = options.CullMode;
	descriptor.primitive.frontFace = options.FrontFace;
	descriptor.multisample = options.MultisampleState;
	if (options.DepthStencilState.format != wgpu::TextureFormat::Undefined)
	{
		descriptor.depthStencil = &options.DepthStencilState;
	}

	renderPipeline = options.Context->Device.CreateRenderPipeline(&descriptor);
}