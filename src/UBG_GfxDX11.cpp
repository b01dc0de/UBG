#include "UBG.h"

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

ID3D11Device* UBG_Gfx_DX11::Device = {};
ID3D11DeviceContext* UBG_Gfx_DX11::Context = {};
D3D_FEATURE_LEVEL UBG_Gfx_DX11::FeatureLevel = {};
IDXGISwapChain1* UBG_Gfx_DX11::SwapChain = {};
ID3D11Texture2D* UBG_Gfx_DX11::BackBuffer = {};
ID3D11RenderTargetView* UBG_Gfx_DX11::RenderTargetView = {};

void GetClearColor(v4f& OutClearColor)
{
    constexpr bool bCycleColorsBG = true;
    if (bCycleColorsBG)
    {
        constexpr v4f Colors[] = {
            { 1.0f, 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f, 0.1f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
        constexpr float StepDurationSeconds = 2.0f;
        constexpr size_t NumColors = ARRAY_SIZE(Colors);

        // TODO: Implement clock time
        float CurrTime = (float)ClockT::CurrTime;
        float Factor = (CurrTime / StepDurationSeconds) - (float)(int)(CurrTime / StepDurationSeconds);
        int StepNumber = (int)(CurrTime / StepDurationSeconds) % NumColors;
        lerp(Colors[StepNumber], Colors[(StepNumber + 1) % NumColors], Factor, OutClearColor);
    }
    else
    {
        OutClearColor = { 242.0f / 255.0f, 80.0f / 255.0f, 34.0f / 255.0f, 1.0f };
    }
}

void UBG_Gfx_DX11::DrawBegin()
{
    Context->OMSetRenderTargets(1, &RenderTargetView, nullptr);

    v4f ClearColor = { };
    GetClearColor(ClearColor);
    Context->ClearRenderTargetView(RenderTargetView, (float*)&ClearColor);
}

void UBG_Gfx_DX11::DrawEnd()
{
    SwapChain->Present(0, 0);
}

bool UBG_Gfx_DX11::Init()
{
    UINT CreateDeviceFlags = 0;
#if _DEBUG
    CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG
    (void)D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        CreateDeviceFlags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &Device,
        &FeatureLevel,
        &Context
    );

    if (!Device)
    {
        Outf("[error] Call to D3D11CreateDevice failed\n");
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 SwapChainDesc1 = {};
    SwapChainDesc1.Width = GlobalState::Width;
    SwapChainDesc1.Height = GlobalState::Height;
    SwapChainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc1.Stereo = FALSE;
    SwapChainDesc1.SampleDesc = { 1, 0 };
    SwapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc1.BufferCount = 2;
    SwapChainDesc1.Scaling = DXGI_SCALING_NONE;
    SwapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDesc1.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    SwapChainDesc1.Flags = 0; // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH

    IDXGIFactory2* DxFactory2 = nullptr;
    (void)CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)(&DxFactory2));
    (void)DxFactory2->CreateSwapChainForHwnd(
        Device,
        // NOTE: This assumes UBG_PlatformT is Win32, which is required for DX anyway...
        UBG_PlatformT::hWindow,
        &SwapChainDesc1,
        nullptr,
        nullptr,
        &SwapChain
    );

    (void)SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);

    (void)Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderTargetView);

    Context->OMSetRenderTargets(1, &RenderTargetView, nullptr);

    return true;
}

bool UBG_Gfx_DX11::Term()
{
    return true;
}

