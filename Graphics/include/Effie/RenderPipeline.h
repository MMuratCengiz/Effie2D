#pragma once

#include "ShaderReflection.h"

namespace Effie
{

struct RenderPipelineOptions
{
	friend class RenderPipeline;

	ShaderReflection * ShaderReflection;
	RenderContext * Context;
	std::vector<wgpu::ColorTargetState> Targets;

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

private:
	RenderPipelineOptions() = default;
};

class RenderPipeline
{
public:
	RenderPipeline(RenderPipelineOptions options);
	GETTER(wgpu::RenderPipeline&, GetInstance, renderPipeline);
private:
	wgpu::RenderPipeline renderPipeline;
	wgpu::FragmentState fragmentState;
	RenderPipelineOptions options;
};

}