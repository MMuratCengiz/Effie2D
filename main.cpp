#include <vector>
#include <string>

#include "dawn/samples/SampleUtils.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/ScopedAutoreleasePool.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"
#include "Effie/RenderDevice.h"
#include "Effie/RenderWindow.h"
#include "Effie/Utilities.h"
#include "tint/source.h"
#include "Effie/ShaderReflection.h"

wgpu::Device device;

wgpu::Buffer indexBuffer;
wgpu::Buffer vertexBuffer;

wgpu::Texture texture;
wgpu::Sampler sampler;

wgpu::Queue queue;
wgpu::SwapChain swapchain;
wgpu::TextureView depthStencilView;
wgpu::RenderPipeline pipeline;
wgpu::BindGroup bindGroup;

void initBuffers()
{
	static const uint32_t indexData[3] = {
		0,
		1,
		2,
	};
	indexBuffer =
		utils::CreateBufferFromData(device, indexData, sizeof(indexData), wgpu::BufferUsage::Index);

	static const float vertexData[12] = {
		0.0f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, 0.0f, 1.0f,
	};
	vertexBuffer = utils::CreateBufferFromData(device, vertexData, sizeof(vertexData),
		wgpu::BufferUsage::Vertex);
}

void initTextures()
{
	wgpu::TextureDescriptor descriptor;
	descriptor.dimension = wgpu::TextureDimension::e2D;
	descriptor.size.width = 1024;
	descriptor.size.height = 1024;
	descriptor.size.depthOrArrayLayers = 1;
	descriptor.sampleCount = 1;
	descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
	descriptor.mipLevelCount = 1;
	descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
	texture = device.CreateTexture(&descriptor);

	sampler = device.CreateSampler();

	// Initialize the texture with arbitrary data until we can load images
	std::vector<uint8_t> data(4 * 1024 * 1024, 0);
	for (size_t i = 0; i < data.size(); ++i)
	{
		data[i] = static_cast<uint8_t>(i % 253);
	}

	wgpu::Buffer stagingBuffer = utils::CreateBufferFromData(
		device, data.data(), static_cast<uint32_t>(data.size()), wgpu::BufferUsage::CopySrc);
	wgpu::ImageCopyBuffer imageCopyBuffer =
		utils::CreateImageCopyBuffer(stagingBuffer, 0, 4 * 1024);
	wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(texture, 0, { 0, 0, 0 });
	wgpu::Extent3D copySize = { 1024, 1024, 1 };

	wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
	encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &copySize);

	wgpu::CommandBuffer copy = encoder.Finish();
	queue.Submit(1, &copy);
}

void init()
{
	device = CreateCppDawnDevice();

	queue = device.GetQueue();
	swapchain = GetSwapChain();

	initBuffers();
	initTextures();

	wgpu::ShaderModule vsModule =
		utils::CreateShaderModule(device, Effie::Utilities::ReadFile("Shaders/Vertex/Sample1.wgsl").c_str());

	wgpu::ShaderModule fsModule =
		utils::CreateShaderModule(device, Effie::Utilities::ReadFile("Shaders/Fragment/Sample1.wgsl").c_str());

	tint::Source::File
		f("Shaders/Fragment/Sample1.wgsl", Effie::Utilities::ReadFile("Shaders/Fragment/Sample1.wgsl").c_str());


	auto bgl = utils::MakeBindGroupLayout(
		device, {
			{ 0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering },
			{ 1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float },
		});

	wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);

	depthStencilView = CreateDefaultDepthStencilView(device);

	utils::ComboRenderPipelineDescriptor descriptor;
	descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);
	descriptor.vertex.module = vsModule;
	descriptor.vertex.bufferCount = 1;

	descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
	descriptor.cBuffers[0].attributeCount = 1;
	descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
	descriptor.cFragment.module = fsModule;
	descriptor.cTargets[0].format = GetPreferredSwapChainTextureFormat();
	descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);

	pipeline = device.CreateRenderPipeline(&descriptor);

	wgpu::TextureView view = texture.CreateView();

	bindGroup = utils::MakeBindGroup(device, bgl, {{ 0, sampler },
												   { 1, view }});
}

int frameIndex = 0;

void frame()
{
	wgpu::TextureView backBufferView = swapchain.GetCurrentTextureView();
	utils::ComboRenderPassDescriptor renderPass({ backBufferView }, depthStencilView);

	wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
	{
		wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
		pass.SetPipeline(pipeline);
		pass.SetBindGroup(0, bindGroup);
		pass.SetVertexBuffer(0, vertexBuffer);
		pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
		pass.DrawIndexed(3);
		pass.End();
	}

	wgpu::CommandBuffer commands = encoder.Finish();
	queue.Submit(1, &commands);
	swapchain.Present();
	DoFlush();
}

int main(int argc, const char* argv[])
{
	Effie::RenderWindow window{ "T1", 1920, 1080 };
	Effie::RenderDevice d{ window.GetSDL_Window() };

//	if (!InitSample(argc, argv))
//	{
//		return 1;
//	}
//
//	init();
//
//	while (!ShouldQuit())
//	{
//		utils::ScopedAutoreleasePool pool;
//		frame();
//		frameIndex = (frameIndex + 1) % 3;
//	}
}
