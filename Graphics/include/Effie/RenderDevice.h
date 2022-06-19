#pragma once

#include "Effie/CommonIncludes.h"
#include "RenderContext.h"

namespace Effie
{

class RenderDevice
{
public:
	RenderDevice(SDL_Window* window);
private:
	wgpu::Surface CreateSurfaceForWindow(const wgpu::Instance& instance, SDL_Window* window);
	std::unique_ptr< wgpu::ChainedStruct > SetupWindowAndGetSurfaceDescriptor(SDL_Window* window);
	std::unique_ptr<RenderContext> context;
};

}