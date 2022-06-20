#pragma once

#include "Effie/CommonIncludes.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/dawn_wsi.h"
#include "dawn/native/DawnNative.h"
#include "dawn/utils/GLFWUtils.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

struct RenderContext
{
	std::unique_ptr<dawn::native::Instance> DawnInstance;
	wgpu::Instance Instance;
	wgpu::Device Device;
	wgpu::SwapChain SwapChain;
	wgpu::Surface Surface;

	DawnProcTable DawnProcTable;
	SDL_Window* Window;
};