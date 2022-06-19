#include "Effie/RenderDevice.h"

using namespace Effie;

RenderDevice::RenderDevice(SDL_Window* window)
{
	context = std::make_unique< RenderContext >();

	ScopedEnvironmentVar angleDefaultPlatform;

	if (GetEnvironmentVar("ANGLE_DEFAULT_PLATFORM").first.empty())
	{
		angleDefaultPlatform.Set("ANGLE_DEFAULT_PLATFORM", "swiftshader");
	}

//	glfwSetErrorCallback(PrintGLFWError);
//	if (!glfwInit())
//	{
//		wgpu::Device();
//	}
//
//	// Create the test window and discover adapters using it (esp. for OpenGL)
//	utils::SetupGLFWWindowHintsForBackend(backendType);
//	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
//	window = glfwCreateWindow(640, 480, "Dawn window", nullptr, nullptr);
//	if (!window)
//	{
//		wgpu::Device();
//	}

	context->DawnInstance = std::make_unique< dawn::native::Instance >();
	context->DawnInstance->DiscoverDefaultAdapters();

	{
		std::vector< dawn::native::Adapter > adapters = context->DawnInstance->GetAdapters();
		auto adapterIt = std::find_if(adapters.begin(), adapters.end(),
			[](const dawn::native::Adapter& adapter) -> bool
			{
			  wgpu::AdapterProperties properties;
			  adapter.GetProperties(&properties);
			  return true;
			});
		ASSERT(adapterIt != adapters.end());
		backendAdapter = *adapterIt;
	}

	WGPUDevice backendDevice = backendAdapter.CreateDevice();
	DawnProcTable backendProcs = dawn::native::GetProcs();

	std::unique_ptr< wgpu::SurfaceDescriptorFromWindowsHWND > desc =
		std::make_unique< wgpu::SurfaceDescriptorFromWindowsHWND >();

	// Create the swapchain
	auto surfaceChainedDesc = utils::SetupWindowAndGetSurfaceDescriptor(window);
	WGPUSurfaceDescriptor surfaceDesc;
	surfaceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(surfaceChainedDesc.get());
	WGPUSurface surface = backendProcs.instanceCreateSurface(context->DawnInstance->Get(), &surfaceDesc);

	WGPUSwapChainDescriptor swapChainDesc;
	swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
	swapChainDesc.format = static_cast<WGPUTextureFormat>(GetPreferredSwapChainTextureFormat());
	swapChainDesc.width = 640;
	swapChainDesc.height = 480;
	swapChainDesc.presentMode = WGPUPresentMode_Mailbox;
	swapChainDesc.implementation = 0;
	WGPUSwapChain backendSwapChain =
		backendProcs.deviceCreateSwapChain(backendDevice, surface, &swapChainDesc);

	// Choose whether to use the backend procs and devices/swapchains directly, or set up the wire.
	WGPUDevice cDevice = nullptr;
	DawnProcTable procs;

	switch (cmdBufType)
	{
	case CmdBufType::None:
		procs = backendProcs;
		cDevice = backendDevice;
		swapChain = wgpu::SwapChain::Acquire(backendSwapChain);
		break;

	case CmdBufType::Terrible:
	{
		c2sBuf = new utils::TerribleCommandBuffer();
		s2cBuf = new utils::TerribleCommandBuffer();

		dawn::wire::WireServerDescriptor serverDesc = { };
		serverDesc.procs = &backendProcs;
		serverDesc.serializer = s2cBuf;

		wireServer = new dawn::wire::WireServer(serverDesc);
		c2sBuf->SetHandler(wireServer);

		dawn::wire::WireClientDescriptor clientDesc = { };
		clientDesc.serializer = c2sBuf;

		wireClient = new dawn::wire::WireClient(clientDesc);
		procs = dawn::wire::client::GetProcs();
		s2cBuf->SetHandler(wireClient);

		auto deviceReservation = wireClient->ReserveDevice();
		wireServer->InjectDevice(backendDevice, deviceReservation.id,
			deviceReservation.generation);
		cDevice = deviceReservation.device;

		auto swapChainReservation = wireClient->ReserveSwapChain(cDevice);
		wireServer->InjectSwapChain(backendSwapChain, swapChainReservation.id,
			swapChainReservation.generation, deviceReservation.id,
			deviceReservation.generation);
		swapChain = wgpu::SwapChain::Acquire(swapChainReservation.swapchain);
	}
		break;
	}

	dawnProcSetProcs(&procs);
	procs.deviceSetUncapturedErrorCallback(cDevice, PrintDeviceError, nullptr);
	context->Device = wgpu::Device::Acquire(cDevice);
}

wgpu::Surface RenderDevice::CreateSurfaceForWindow(const wgpu::Instance& instance, SDL_Window* window) {
	std::unique_ptr<wgpu::ChainedStruct> chainedDescriptor =
		SetupWindowAndGetSurfaceDescriptor(window);

	wgpu::SurfaceDescriptor descriptor;
	descriptor.nextInChain = chainedDescriptor.get();
	wgpu::Surface surface = instance.CreateSurface(&descriptor);

	return surface;
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
	return SetupWindowAndGetSurfaceDescriptorCocoa(window);
#else
	return nullptr;
#endif
}