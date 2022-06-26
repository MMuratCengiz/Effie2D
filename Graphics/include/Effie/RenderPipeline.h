#pragma once

#include "ShaderReflection.h"

namespace Effie
{

struct RenderPipelineOptions
{
	ShaderReflection * ShaderReflection;
	RenderContext * Context;

	wgpu::CullMode CullMode = wgpu::CullMode::None;
	wgpu::FrontFace FrontFace = wgpu::FrontFace::CCW;
	wgpu::MultisampleState MultisampleState;
	wgpu::DepthStencilState DepthStencilState;

	RenderPipelineOptions(RenderContext * context, class ShaderReflection * shaderReflection)
	{
		DepthStencilState.format = wgpu::TextureFormat::Undefined;
		ShaderReflection = shaderReflection;
		Context = context;
	}
};

class RenderPipeline
{
public:
	RenderPipeline(const RenderPipelineOptions& options);
	GETTER(wgpu::RenderPipeline&, GetInstance, renderPipeline);
private:
	wgpu::RenderPipeline renderPipeline;
};

}