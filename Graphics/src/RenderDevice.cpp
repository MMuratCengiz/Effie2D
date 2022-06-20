#include "Effie/RenderDevice.h"

using namespace Effie;

void LogDeviceError(WGPUErrorType errorType, const char* message, void*)
{
	std::string errorTypeName;

	switch (errorType)
	{
	case WGPUErrorType_Validation:
		errorTypeName = "Validation";
		break;
	case WGPUErrorType_OutOfMemory:
		errorTypeName = "Out of memory";
		break;
	case WGPUErrorType_Unknown:
		errorTypeName = "Unknown";
		break;
	case WGPUErrorType_DeviceLost:
		errorTypeName = "Device lost";
		break;
	default:
		return;
	}

	LOG(ERROR) << errorTypeName << " error: " << message;
}

RenderDevice::RenderDevice(SDL_Window* window)
{
	context = std::make_unique< RenderContext >();
	context->Window = window;

	ScopedEnvironmentVar angleDefaultPlatform;

	if (GetEnvironmentVar("ANGLE_DEFAULT_PLATFORM").first.empty())
	{
		angleDefaultPlatform.Set("ANGLE_DEFAULT_PLATFORM", "swiftshader");
	}

	context->DawnInstance = std::make_unique< dawn::native::Instance >();
	context->DawnInstance->DiscoverDefaultAdapters();

	dawn::native::Adapter fallbackAdapter;
	dawn::native::Adapter backendAdapter;

	std::vector< dawn::native::Adapter > adapters = context->DawnInstance->GetAdapters();
	ASSERT_TRUE(adapters.empty(), "No suitable adapter found.");

	auto adapterIt = std::find_if(
		adapters.begin(),
		adapters.end(),
		[](const dawn::native::Adapter& adapter)
		{
		  wgpu::AdapterProperties properties;
		  adapter.GetProperties(&properties);
		  return properties.adapterType == wgpu::AdapterType::DiscreteGPU;
		}
	);

	backendAdapter = adapterIt == adapters.end() ? adapters[0] : *adapterIt;

	WGPUDevice backendDevice = backendAdapter.CreateDevice();
	context->Device = wgpu::Device::Acquire(backendDevice);

	context->DawnProcTable = dawn::native::GetProcs();

	std::unique_ptr< wgpu::SurfaceDescriptorFromWindowsHWND > desc =
		std::make_unique< wgpu::SurfaceDescriptorFromWindowsHWND >();

	auto surfaceChainedDesc = SetupWindowAndGetSurfaceDescriptor(window);
	WGPUSurfaceDescriptor surfaceDesc;
	surfaceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(surfaceChainedDesc.get());
	auto surface = context->DawnProcTable.instanceCreateSurface(context->DawnInstance->Get(), &surfaceDesc);
	context->Surface = wgpu::Surface::Acquire(surface);

	CreateSwapChain();

	dawnProcSetProcs(&context->DawnProcTable);
	context->DawnProcTable.deviceSetUncapturedErrorCallback(backendDevice, LogDeviceError, nullptr);
}

void RenderDevice::CreateSwapChain()
{
	if (context->SwapChain != nullptr)
	{
		context->SwapChain.Release();
	}

	int w, h;
	SDL_GetWindowSize(context->Window, &w, &h);
	WGPUSwapChainDescriptor swapChainDesc;
	swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
	swapChainDesc.format = static_cast<WGPUTextureFormat>(wgpu::TextureFormat::BGRA8Unorm);
	swapChainDesc.width = w;
	swapChainDesc.height = h;
	swapChainDesc.presentMode = WGPUPresentMode_Mailbox;
	swapChainDesc.implementation = 0;

	WGPUSwapChain backendSwapChain =
		context->DawnProcTable.deviceCreateSwapChain(context->Device.Get(), context->Surface.Get(), &swapChainDesc);

	context->SwapChain = wgpu::SwapChain::Acquire(backendSwapChain);
}

std::unique_ptr< wgpu::ChainedStruct > RenderDevice::SetupWindowAndGetSurfaceDescriptor(SDL_Window* window)
{
#if DAWN_PLATFORM_IS(WINDOWS)
	std::unique_ptr< wgpu::SurfaceDescriptorFromWindowsHWND > desc =
		std::make_unique< wgpu::SurfaceDescriptorFromWindowsHWND >();
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);

	desc->hwnd = wmInfo.info.win.window;
	desc->hinstance = wmInfo.info.win.hinstance;
	return std::move(desc);
#elif defined(DAWN_ENABLE_BACKEND_METAL)
	std::unique_ptr<wgpu::SurfaceDescriptorFromMetalLayer> desc =
		std::make_unique<wgpu::SurfaceDescriptorFromMetalLayer>();

	auto metalView = SDL_Metal_CreateView(window);
	desc->layer = SDL_Metal_GetLayer(metalView);

	return std::move(desc);
#else
	return nullptr;
#endif
}