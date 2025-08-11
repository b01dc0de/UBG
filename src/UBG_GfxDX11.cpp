#include "UBG.h"

// NOTE: This include lib format is supported only via MSVC, keep in mind if we want to support other compilers on Windows
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "DXGI.lib")

ID3D11Device* UBG_Gfx_DX11::Device = {};
ID3D11DeviceContext* UBG_Gfx_DX11::Context = {};
D3D_FEATURE_LEVEL UBG_Gfx_DX11::FeatureLevel = {};
IDXGISwapChain1* UBG_Gfx_DX11::SwapChain = {};
ID3D11Texture2D* UBG_Gfx_DX11::BackBuffer = {};
ID3D11RenderTargetView* UBG_Gfx_DX11::RenderTargetView = {};
ID3D11RasterizerState* UBG_Gfx_DX11::RasterState = {};
ID3D11Texture2D* UBG_Gfx_DX11::DepthStencil = {};
ID3D11DepthStencilView* UBG_Gfx_DX11::DepthStencilView = {};

void SafeRelease(IUnknown* Ptr)
{
    if (Ptr)
    {
        Ptr->Release();
    }
}

struct DrawStateT
{
    ID3D11InputLayout* InputLayout;
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;

    void Bind(ID3D11DeviceContext* Context)
    {
        Context->IASetInputLayout(InputLayout);
        Context->VSSetShader(VertexShader, nullptr, 0);
        Context->PSSetShader(PixelShader, nullptr, 0);
    }

    void SafeRelease()
    {
        ::SafeRelease(InputLayout);
        ::SafeRelease(VertexShader);
        ::SafeRelease(PixelShader);
    }
};

struct MeshStateT
{
    size_t VertexSize;
    size_t NumVerts;
    size_t NumInds;
    ID3D11Buffer* VxBuffer;
    ID3D11Buffer* IxBuffer;

    void Bind(ID3D11DeviceContext* Context)
    {
        UINT VxStride = (UINT)VertexSize;
        UINT VxOffset = 0u;
        Context->IASetVertexBuffers(0, 1, &VxBuffer, &VxStride, &VxOffset);
        Context->IASetIndexBuffer(IxBuffer, DXGI_FORMAT_R32_UINT, 0);
        Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void SafeRelease()
    {
        ::SafeRelease(VxBuffer);
        ::SafeRelease(IxBuffer);
    }
};

struct Camera
{
    m4f View;
    m4f Proj;

    void Ortho(float ResX, float ResY, float fDepth)
    {
        View = m4f::Identity();
        Proj = m4f::Identity();

        Proj.V0.X = +2.0f / ResX;
        Proj.V1.Y = +2.0f / ResY;
        View.V2.Z = -2.0f / fDepth;
    }
};

struct GfxPrivData
{
    static ID3D11Texture2D* DefaultTexture;
    static ID3D11ShaderResourceView* DefaultTextureSRV;
    static ID3D11SamplerState* DefaultSamplerState;
    static ID3D11Buffer* WorldBuffer;
    static ID3D11Buffer* ViewProjBuffer;

    static DrawStateT DrawColor;
    static DrawStateT DrawTexture;
    static DrawStateT DrawUnicolor;
    static ID3D11Buffer* UnicolorBuffer;

    static MeshStateT MeshTriangle;
    static MeshStateT MeshQuad;
    static MeshStateT MeshQuadMin;

    static Camera CameraO;

    static void Draw(ID3D11DeviceContext* Context);
    static bool Init(ID3D11Device* Device);
    static bool Term();
};

ID3D11Texture2D* GfxPrivData::DefaultTexture = {};
ID3D11ShaderResourceView* GfxPrivData::DefaultTextureSRV = {};
ID3D11SamplerState* GfxPrivData::DefaultSamplerState = {};
ID3D11Buffer* GfxPrivData::WorldBuffer = {};
ID3D11Buffer* GfxPrivData::ViewProjBuffer = {};

DrawStateT GfxPrivData::DrawColor = {};
DrawStateT GfxPrivData::DrawTexture = {};
DrawStateT GfxPrivData::DrawUnicolor = {};
ID3D11Buffer* GfxPrivData::UnicolorBuffer = {};
MeshStateT GfxPrivData::MeshTriangle = {};
MeshStateT GfxPrivData::MeshQuad = {};
MeshStateT GfxPrivData::MeshQuadMin = {};

Camera GfxPrivData::CameraO = {};

v4f GetRandomColorDim();
void GetClearColor(v4f& OutClearColor);
int CompileShaderHLSL
(
    const wchar_t* SourceFileName,
    LPCSTR EntryPointFunction,
    LPCSTR ShaderProfile,
    ID3DBlob** OutShaderBlob,
    const D3D_SHADER_MACRO* Defines
);
DrawStateT CreateDrawState
(
    ID3D11Device* Device,
    const wchar_t* ShaderFileName,
    const D3D_SHADER_MACRO* Defines,
    const D3D11_INPUT_ELEMENT_DESC* InputElements,
    size_t NumInputElements
);
MeshStateT CreateMeshState
(
    ID3D11Device* InDevice,
    size_t VertexSize,
    size_t NumVertices,
    void* VertexData,
    size_t NumIndices,
    unsigned int* IndexData
);

void GfxPrivData::Draw(ID3D11DeviceContext* Context)
{
    ID3D11Buffer* WVPBuffers[] = {WorldBuffer, ViewProjBuffer};
    float HalfWidth = GlobalState::Width * 0.5f;
    float HalfHeight = GlobalState::Height * 0.5f;
    m4f SpriteWorld = m4f::Scale(HalfWidth, HalfHeight, 1.0f) * m4f::Trans(0.0f, 0.0f, 0.0f);
    Context->UpdateSubresource(WorldBuffer, 0, nullptr, &SpriteWorld, (UINT)sizeof(m4f), 0);
    Context->UpdateSubresource(ViewProjBuffer, 0, nullptr, &CameraO, (UINT)sizeof(CameraO), 0);
    
    // Draw MeshTriangle using DrawColor:
    {
        MeshTriangle.Bind(Context);

        DrawColor.Bind(Context);
        Context->VSSetConstantBuffers(0, ARRAY_SIZE(WVPBuffers), WVPBuffers);
        Context->PSSetConstantBuffers(0, ARRAY_SIZE(WVPBuffers), WVPBuffers);

        UINT StartIdx = 0;
        UINT StartVx = 0;
        Context->DrawIndexed((UINT)MeshTriangle.NumInds, StartIdx, StartVx);
    }

    // Draw MeshQuad using DrawTexture:
    {
        MeshQuad.Bind(Context);

        DrawTexture.Bind(Context);
        Context->VSSetShaderResources(0, 1, &DefaultTextureSRV);
        Context->PSSetShaderResources(0, 1, &DefaultTextureSRV);
        Context->VSSetSamplers(0, 1, &DefaultSamplerState);
        Context->PSSetSamplers(0, 1, &DefaultSamplerState);
        Context->VSSetConstantBuffers(0, ARRAY_SIZE(WVPBuffers), WVPBuffers);
        Context->PSSetConstantBuffers(0, ARRAY_SIZE(WVPBuffers), WVPBuffers);

        UINT StartIdx = 0;
        UINT StartVx = 0;
        Context->DrawIndexed((UINT)MeshQuad.NumInds, StartIdx, StartVx);
    }

    // Draw MeshQuadMin using DrawUnicolor:
    {
        MeshQuadMin.Bind(Context);

        DrawUnicolor.Bind(Context);
        v4f UnicolorData[4] = { };
        GetClearColor(UnicolorData[0]);
        // Set it to opposite color of clear color for now
        UnicolorData[0] = { 1.0f - UnicolorData[0].X, 1.0f - UnicolorData[0].Y, 1.0f - UnicolorData[0].Z, 1.0f };
        Context->UpdateSubresource(UnicolorBuffer, 0, nullptr, UnicolorData, (UINT)sizeof(UnicolorData), 0);
        ID3D11Buffer* UnicolorShaderBuffers[] = { WorldBuffer, ViewProjBuffer, UnicolorBuffer };
        Context->VSSetConstantBuffers(0, ARRAY_SIZE(UnicolorShaderBuffers), UnicolorShaderBuffers);
        Context->PSSetConstantBuffers(0, ARRAY_SIZE(UnicolorShaderBuffers), UnicolorShaderBuffers);

        UINT StartIdx = 0;
        UINT StartVx = 0;
        Context->DrawIndexed((UINT)MeshQuadMin.NumInds, StartIdx, StartVx);
    }
}

bool GfxPrivData::Init(ID3D11Device* Device)
{
    // Default texture / sampler:
    {
        ImageT DefaultImage = {};
        GetDebugImage(DefaultImage);

        D3D11_SUBRESOURCE_DATA TextureResDataDesc[] = { {} };
        TextureResDataDesc[0].pSysMem = DefaultImage.PxBuffer;
        TextureResDataDesc[0].SysMemPitch = sizeof(RGBA32) * DefaultImage.Width;
        TextureResDataDesc[0].SysMemSlicePitch = TextureResDataDesc[0].SysMemPitch * DefaultImage.Height;
        D3D11_TEXTURE2D_DESC DefaultTextureDesc = {};
        DefaultTextureDesc.Width = DefaultImage.Width;
        DefaultTextureDesc.Height = DefaultImage.Height;
        DefaultTextureDesc.MipLevels = 1;
        DefaultTextureDesc.ArraySize = 1;
        DefaultTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        DefaultTextureDesc.SampleDesc = { 1, 0 };
        DefaultTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        DefaultTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        Device->CreateTexture2D(&DefaultTextureDesc, &TextureResDataDesc[0], &DefaultTexture);
        Device->CreateShaderResourceView(DefaultTexture, nullptr, &DefaultTextureSRV);

        D3D11_SAMPLER_DESC DefaultSamplerDesc = {};
        DefaultSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        DefaultSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        DefaultSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        DefaultSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        Device->CreateSamplerState(&DefaultSamplerDesc, &DefaultSamplerState);

        delete[] DefaultImage.PxBuffer;
    }

    D3D_SHADER_MACRO DefaultDefines[] =
    {
        "ENABLE_WVP_TRANSFORM", "1",
        nullptr, nullptr
    };

    // World / ViewProj cbuffers
    {
        D3D11_BUFFER_DESC CommonBufferDesc = {};
        CommonBufferDesc.ByteWidth = sizeof(m4f);
        CommonBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        CommonBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        CommonBufferDesc.CPUAccessFlags = 0;
        Device->CreateBuffer(&CommonBufferDesc, nullptr, &WorldBuffer);
        CommonBufferDesc.ByteWidth = sizeof(m4f) * 2;
        Device->CreateBuffer(&CommonBufferDesc, nullptr, &ViewProjBuffer);
    }

    // Shader color:
    {
        D3D11_INPUT_ELEMENT_DESC InputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        DrawColor = CreateDrawState
        (
            Device,
            L"src/hlsl/BaseShaderColor.hlsl",
            DefaultDefines,
            InputElements,
            ARRAY_SIZE(InputElements)
        );
    }

    // Shader texture:
    {
        D3D11_INPUT_ELEMENT_DESC InputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        DrawTexture = CreateDrawState
        (
            Device,
            L"src/hlsl/BaseShaderTexture.hlsl",
            DefaultDefines,
            InputElements,
            ARRAY_SIZE(InputElements)
        );
    }

    // Shader unicolor:
    {
        D3D11_INPUT_ELEMENT_DESC InputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        DrawUnicolor = CreateDrawState
        (
            Device,
            L"src/hlsl/BaseShaderUnicolor.hlsl",
            DefaultDefines,
            InputElements,
            ARRAY_SIZE(InputElements)
        );

        D3D11_BUFFER_DESC UnicolorBufferDesc = {};
        UnicolorBufferDesc.ByteWidth = sizeof(v4f);
        UnicolorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        UnicolorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        UnicolorBufferDesc.CPUAccessFlags = 0;
        Device->CreateBuffer(&UnicolorBufferDesc, nullptr, &UnicolorBuffer);
    }

    // Mesh triangle:
    {
        VxColor TriangleVerts[] = {
            { { 0.0f, 0.5f, 0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { -0.5f, -0.5f, 0.5f, 1.0f}, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { +0.5f, -0.5f, 0.5f, 1.0f}, { 0.0f, 0.0f, 1.0f, 1.0f } },
        };

        unsigned int TriangleInds[] = { 0, 2, 1 };

        MeshTriangle = CreateMeshState
        (
            Device,
            sizeof(VxColor),
            ARRAY_SIZE(TriangleVerts),
            TriangleVerts,
            ARRAY_SIZE(TriangleInds),
            TriangleInds
        );
    }

    // Mesh(es) quad
    {
        VxTex QuadVerts[] = {
            { { -0.5f, +0.5f, +0.5f, 1.0f}, { 0.0f, 0.0f } },
            { { +0.5f, +0.5f, +0.5f, 1.0f}, { 1.0f, 0.0f } },
            { { -0.5f, -0.5f, +0.5f, 1.0f}, { 0.0f, 1.0f } },
            { { +0.5f, -0.5f, +0.5f, 1.0f}, { 1.0f, 1.0f } },
        };

        // TODO: Remove this once we have WVP transforms implemented, this is just to not draw the tex quad + unicolor quad in the same place
        constexpr bool bTODORemoveThisTempQuadOffsetLater = true;
        if (bTODORemoveThisTempQuadOffsetLater)
        {
            for (int VxIdx = 0; VxIdx < ARRAY_SIZE(QuadVerts); VxIdx++)
            {
                QuadVerts[VxIdx].Pos.X -= 0.5f;
                QuadVerts[VxIdx].Pos.Y -= 0.5f;
            }
        }

        unsigned int QuadInds[] = { 0, 1, 2,    1, 3, 2 };

        MeshQuad = CreateMeshState
        (
            Device,
            sizeof(VxTex),
            ARRAY_SIZE(QuadVerts),
            QuadVerts,
            ARRAY_SIZE(QuadInds),
            QuadInds
        );

        VxMin QuadMinVerts[] = {
            { -0.5f, +0.5f, +0.5f, 1.0f},
            { +0.5f, +0.5f, +0.5f, 1.0f},
            { -0.5f, -0.5f, +0.5f, 1.0f},
            { +0.5f, -0.5f, +0.5f, 1.0f},
        };

        MeshQuadMin = CreateMeshState
        (
            Device,
            sizeof(VxMin),
            ARRAY_SIZE(QuadMinVerts),
            QuadMinVerts,
            ARRAY_SIZE(QuadInds),
            QuadInds
        );

    }

    // TODO: Why is Depth passed as -2?
    CameraO.Ortho((float)GlobalState::Width, (float)GlobalState::Height, -2.0f);
    
    // TODO: Check for correct init'd state here
    return true;
}

bool GfxPrivData::Term()
{
    SafeRelease(DefaultTexture);
    SafeRelease(DefaultTextureSRV);
    SafeRelease(DefaultSamplerState);
    SafeRelease(WorldBuffer);
    SafeRelease(ViewProjBuffer);

    DrawColor.SafeRelease();
    DrawTexture.SafeRelease();
    DrawUnicolor.SafeRelease();
    SafeRelease(UnicolorBuffer);

    MeshTriangle.SafeRelease();
    MeshQuad.SafeRelease();
    MeshQuadMin.SafeRelease();

    return true;
}

void UBG_Gfx_DX11::DrawBegin()
{
    Context->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);

    v4f ClearColor = { };
    GetClearColor(ClearColor);
    Context->ClearRenderTargetView(RenderTargetView, (float*)&ClearColor);
    Context->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void UBG_Gfx_DX11::DrawEnd()
{
    UINT SyncInterval = 0;
    UINT PresentFlags = 0;
    DXGI_PRESENT_PARAMETERS PresentParams = {};
    SwapChain->Present1(SyncInterval, PresentFlags, &PresentParams);
}

void UBG_Gfx_DX11::Draw()
{
    DrawBegin();
    GfxPrivData::Draw(Context);
    DrawEnd();
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
    //SwapChainDesc1.Scaling = DXGI_SCALING_NONE;
    SwapChainDesc1.Scaling = DXGI_SCALING_STRETCH;
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

    D3D11_TEXTURE2D_DESC DepthDesc = {};
    DepthDesc.Width = GlobalState::Width;
    DepthDesc.Height = GlobalState::Height;
    DepthDesc.MipLevels = 1;
    DepthDesc.ArraySize = 1;
    DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthDesc.SampleDesc = { 1, 0 };
    DepthDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DepthDesc.CPUAccessFlags = 0;
    DepthDesc.MiscFlags = 0;
    Device->CreateTexture2D(&DepthDesc, nullptr, &DepthStencil);

    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
    DepthStencilViewDesc.Format = DepthDesc.Format;
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DepthStencilViewDesc.Texture2D.MipSlice = 0;
    Device->CreateDepthStencilView(DepthStencil, &DepthStencilViewDesc, &DepthStencilView);

    D3D11_RASTERIZER_DESC RasterDesc = {};
    RasterDesc.FillMode = D3D11_FILL_SOLID;
    RasterDesc.CullMode = D3D11_CULL_BACK;
    RasterDesc.FrontCounterClockwise = TRUE;
    RasterDesc.DepthClipEnable = TRUE;
    RasterDesc.ScissorEnable = FALSE;
    RasterDesc.MultisampleEnable = TRUE;
    RasterDesc.AntialiasedLineEnable = TRUE;
    Device->CreateRasterizerState(&RasterDesc, &RasterState);

    D3D11_VIEWPORT ViewportDesc = {};
    ViewportDesc.TopLeftX = 0.0f;
    ViewportDesc.TopLeftY = 0.0f;
    ViewportDesc.Width = (FLOAT)GlobalState::Width;
    ViewportDesc.Height = (FLOAT)GlobalState::Height;
    ViewportDesc.MinDepth = 0.0f;
    ViewportDesc.MaxDepth = 1.0f;
    Context->RSSetViewports(1, &ViewportDesc);

    Context->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);


    bool bResult = Device && Context &&
        SwapChain && BackBuffer &&
        RenderTargetView && RasterState &&
        DepthStencil && DepthStencilView;

    bResult &= GfxPrivData::Init(Device);

    ASSERT(bResult);

    return bResult;
}

bool UBG_Gfx_DX11::Term()
{
    SafeRelease(DepthStencil);
    SafeRelease(DepthStencilView);

    SafeRelease(RenderTargetView);
    SafeRelease(RasterState);

    SafeRelease(SwapChain);
    SafeRelease(BackBuffer);

    SafeRelease(Context);
#if _DEBUG
    if (Device)
    {
        ID3D11Debug* DxDebug = nullptr;
        Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&DxDebug);
        if (DxDebug) { DxDebug->ReportLiveDeviceObjects(D3D11_RLDO_IGNORE_INTERNAL); }
        SafeRelease(DxDebug);
    }
#endif // _DEBUG
    SafeRelease(Device);

    return true;
}

v4f GetRandomColorDim()
{
    v4f Result = {};
    Result.X = GetRandomInt(0, 100) / 255.0f;
    Result.Y = GetRandomInt(0, 100) / 255.0f;
    Result.Z = GetRandomInt(0, 100) / 255.0f;
    Result.W = 1.0f;
    return Result;
}

void GetClearColor(v4f& OutClearColor)
{
    enum struct ClearColorMode
    {
        SOLID,
        CYCLE_COLORS,
        CYCLE_RANDOM,
    };
    constexpr ClearColorMode Mode = ClearColorMode::CYCLE_RANDOM;
    switch (Mode)
    {
        case ClearColorMode::SOLID:
        {
            OutClearColor = { 242.0f / 255.0f, 80.0f / 255.0f, 34.0f / 255.0f, 1.0f };
        } break;

        case ClearColorMode::CYCLE_COLORS:
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

            float CurrTime = (float)ClockT::CurrTime;
            float Factor = (CurrTime / StepDurationSeconds) - (float)(int)(CurrTime / StepDurationSeconds);
            int StepNumber = (int)(CurrTime / StepDurationSeconds) % NumColors;
            OutClearColor = lerp(Colors[StepNumber], Colors[(StepNumber + 1) % NumColors], Factor);
        } break;

        case ClearColorMode::CYCLE_RANDOM:
        {
            static v4f CurrColor = GetRandomColorDim();
            static v4f NextColor = GetRandomColorDim();
            static float LastSwitchTime = 0.0f;
            constexpr float StepDurationSeconds = 2.0f;

            float CurrTime = (float)ClockT::CurrTime;
            if (CurrTime - LastSwitchTime > StepDurationSeconds)
            {
                LastSwitchTime = CurrTime;
                CurrColor = NextColor;
                NextColor = GetRandomColorDim();
            }
            float Factor = (CurrTime - LastSwitchTime) / StepDurationSeconds;
            OutClearColor = lerp(CurrColor, NextColor, Factor);
        } break;

        default:
        {
            OutClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        } break;
    }
}

int CompileShaderHLSL
(
    const wchar_t* SourceFileName,
    LPCSTR EntryPointFunction,
    LPCSTR ShaderProfile,
    ID3DBlob** OutShaderBlob,
    const D3D_SHADER_MACRO* Defines
)
{
    if (SourceFileName == nullptr || EntryPointFunction == nullptr || ShaderProfile == nullptr || OutShaderBlob == nullptr)
    {
        return E_INVALIDARG;
    }

    HRESULT Result = S_OK;

    UINT ShaderCompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
    ShaderCompileFlags |= D3DCOMPILE_DEBUG;
    ShaderCompileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

    ID3DBlob* ShaderBlob = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    // TODO: Switch to using D3DCompile2(...) instead of D3DCompileFromFile
    Result = D3DCompileFromFile
    (
        SourceFileName,
        Defines,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        EntryPointFunction,
        ShaderProfile,
        ShaderCompileFlags,
        0,
        &ShaderBlob,
        &ErrorBlob
    );

    if (FAILED(Result) && ShaderBlob)
    {
        ShaderBlob->Release();
        ShaderBlob = nullptr;
    }
    if (ErrorBlob)
    {
        OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
        ErrorBlob->Release();
    }

    *OutShaderBlob = ShaderBlob;

    return Result;
}

DrawStateT CreateDrawState
(
    ID3D11Device* Device,
    const wchar_t* ShaderFileName,
    const D3D_SHADER_MACRO* Defines,
    const D3D11_INPUT_ELEMENT_DESC* InputElements,
    size_t NumInputElements
)
{
    ASSERT(Device);
    ASSERT(ShaderFileName);
    ASSERT(InputElements);

    ID3DBlob* VSBlob = nullptr;
    ID3DBlob* PSBlob = nullptr;

    static const char* VxShaderMain = "VSMain";
    static const char* VxShaderProfile = "vs_5_0";
    static const char* PxShaderMain = "PSMain";
    static const char* PxShaderProfile = "ps_5_0";
    CompileShaderHLSL(ShaderFileName, VxShaderMain, VxShaderProfile, &VSBlob, Defines);
    CompileShaderHLSL(ShaderFileName, PxShaderMain, PxShaderProfile, &PSBlob, Defines);

    DrawStateT Result;

    if (VSBlob && PSBlob)
    {
        Device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &Result.VertexShader);
        Device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &Result.PixelShader);

        Device->CreateInputLayout(InputElements, (UINT)NumInputElements, VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &Result.InputLayout);
    }
    SafeRelease(VSBlob);
    SafeRelease(PSBlob);

    return Result;
}

MeshStateT CreateMeshState
(
    ID3D11Device* InDevice,
    size_t VertexSize,
    size_t NumVertices,
    void* VertexData,
    size_t NumIndices,
    unsigned int* IndexData
)
{
    ASSERT(VertexData);

    MeshStateT Result;

    Result.VertexSize = VertexSize;
    Result.NumVerts = NumVertices;
    Result.NumInds = NumIndices;

    size_t VxDataSize = VertexSize * NumVertices;
    D3D11_BUFFER_DESC VertexBufferDesc = { (UINT)VxDataSize, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0 };
    D3D11_SUBRESOURCE_DATA VertexBufferInitData = { VertexData, 0, 0 };
    InDevice->CreateBuffer(&VertexBufferDesc, &VertexBufferInitData, &Result.VxBuffer);

    if (IndexData)
    {
        size_t IxDataSize = NumIndices * sizeof(unsigned int);
        D3D11_BUFFER_DESC IndexBufferDesc = { (UINT)IxDataSize, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0 };
        D3D11_SUBRESOURCE_DATA IndexBufferInitData = { IndexData, 0, 0 };
        InDevice->CreateBuffer(&IndexBufferDesc, &IndexBufferInitData, &Result.IxBuffer);
    }

    return Result;
}

