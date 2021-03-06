#pragma once

#include "Effie/CommonIncludes.h"
#include "RenderContext.h"

namespace Effie
{

class RenderDevice
{
public:
	explicit RenderDevice(SDL_Window* window);
	~RenderDevice();

	GETTER(RenderContext*, GetContext, context.get())
private:
	std::unique_ptr< wgpu::ChainedStruct > SetupWindowAndGetSurfaceDescriptor(SDL_Window* window);
	void CreateSwapChain();

	std::unique_ptr<RenderContext> context;
};

}