/*
 *  Copyright 2019-2022 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#include <memory>


#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#undef main

#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

#include "Common/interface/RefCntAutoPtr.hpp"

using namespace Diligent;

static const char *VSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};
void main(in  uint    VertId : SV_VertexID,
          out PSInput PSIn) 
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);
    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue
    PSIn.Pos   = Pos[VertId];
    PSIn.Color = Col[VertId];
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char *PSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};
struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};
void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = float4(PSIn.Color.rgb, 1.0);
}
)";


class Tutorial00App
{
public:
    Tutorial00App( ) = default;

    ~Tutorial00App( )
    {
        m_pImmediateContext->Flush( );
    }

    void InitializeDiligentEngine( HWND hWnd )
    {
        SwapChainDesc SCDesc;
        SCDesc.BufferCount = 3;
        SCDesc.ColorBufferFormat = Diligent::TEX_FORMAT_BGRA8_UNORM;
        auto GetEngineFactoryVk = LoadGraphicsEngineVk( );
        EngineVkCreateInfo EngineCI;

        auto *pFactoryVk = GetEngineFactoryVk( );
        pFactoryVk->CreateDeviceAndContextsVk( EngineCI, &m_pDevice, &m_pImmediateContext );

        if ( !m_pSwapChain && hWnd != nullptr )
        {
            Win32NativeWindow Window { hWnd };
            pFactoryVk->CreateSwapChainVk( m_pDevice, m_pImmediateContext, SCDesc, Window, &m_pSwapChain );
        }
    }

    void CreateResources( )
    {
        // Pipeline state object encompasses configuration of all GPU stages
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        PSOCreateInfo.PSODesc.Name = "Simple triangle PSO";

        // This is a graphics pipeline
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
        // Set render target format which is the format of the swap chain's color buffer
        PSOCreateInfo.GraphicsPipeline.RTVFormats[ 0 ] = m_pSwapChain->GetDesc( ).ColorBufferFormat;
        // Use the depth buffer format from the swap chain
        PSOCreateInfo.GraphicsPipeline.DSVFormat = m_pSwapChain->GetDesc( ).DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // No back face culling for this tutorial
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        // Disable depth testing
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        // clang-format on

        ShaderCreateInfo ShaderCI;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        ShaderCI.UseCombinedTextureSamplers = true;
        // Create a vertex shader
        RefCntAutoPtr< IShader > pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle vertex shader";
            ShaderCI.Source = VSSource;
            m_pDevice->CreateShader( ShaderCI, &pVS );
        }

        // Create a pixel shader
        RefCntAutoPtr< IShader > pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle pixel shader";
            ShaderCI.Source = PSSource;
            m_pDevice->CreateShader( ShaderCI, &pPS );
        }

        // Finally, create the pipeline state
        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;
        m_pDevice->CreateGraphicsPipelineState( PSOCreateInfo, &m_pPSO );
    }

    void Render( )
    {
        // Set render targets before issuing any draw command.
        // Note that Present() unbinds the back buffer if it is set as render target.
        auto *pRTV = m_pSwapChain->GetCurrentBackBufferRTV( );
        auto *pDSV = m_pSwapChain->GetDepthBufferDSV( );
        m_pImmediateContext->SetRenderTargets( 1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

        // Clear the back buffer
        const float ClearColor[] = { 0.350f, 0.350f, 0.350f, 1.0f };
        // Let the engine perform required state transitions
        m_pImmediateContext->ClearRenderTarget( pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
        m_pImmediateContext->ClearDepthStencil( pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

        // Set the pipeline state in the immediate context
        m_pImmediateContext->SetPipelineState( m_pPSO );
        // Typically we should now call CommitShaderResources(), however shaders in this example don't
        // use any resources.
        DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // Render 3 vertices
        m_pImmediateContext->Draw( drawAttrs );
    }

    void Present( )
    {
        m_pSwapChain->Present( 0 );
    }

    void WindowResize( Uint32 Width, Uint32 Height )
    {
        if ( m_pSwapChain )
            m_pSwapChain->Resize( Width, Height );
    }

    [[nodiscard]] RENDER_DEVICE_TYPE GetDeviceType( ) const
    { return m_DeviceType; }

private:
    RefCntAutoPtr< IRenderDevice > m_pDevice;
    RefCntAutoPtr< IDeviceContext > m_pImmediateContext;
    RefCntAutoPtr< ISwapChain > m_pSwapChain;
    RefCntAutoPtr< IPipelineState > m_pPSO;
    RENDER_DEVICE_TYPE m_DeviceType = RENDER_DEVICE_TYPE_D3D11;
};

std::unique_ptr< Tutorial00App > g_pTheApp;

int main( )
{
    SDL_Init( SDL_INIT_EVERYTHING );

    auto window_flags = ( SDL_WindowFlags ) ( SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN );

    auto window = SDL_CreateWindow(
            "LostGrip",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            1920,
            1080,
            window_flags
    );

    if ( window == nullptr )
    {
        std::cout << SDL_GetError( ) << std::endl;
        return -1;
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION( &wmInfo.version );
    SDL_GetWindowWMInfo( window, &wmInfo );
    HWND wnd = wmInfo.info.win.window;

    g_pTheApp = std::make_unique< Tutorial00App >( );

    g_pTheApp->InitializeDiligentEngine( wnd );
    g_pTheApp->CreateResources( );

    SDL_Event event { };
    while ( event.type != SDL_QUIT )
    {
        SDL_PollEvent( &event );
        if ( event.type == SDL_WINDOWEVENT_RESIZED )
        {
            g_pTheApp->WindowResize( event.window.data1, event.window.data2 );
        }

        g_pTheApp->Render( );
        g_pTheApp->Present( );
    }

    g_pTheApp.reset( );
    SDL_DestroyWindow( window );

    return 0;
}