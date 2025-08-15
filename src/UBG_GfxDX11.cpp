#include "UBG.h" // E UBG_Gfx.h E UBG_GfxDX11.h

// NOTE: This include lib format is supported only via MSVC, keep in mind if we want to support other compilers on Windows
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "DXGI.lib")

void SafeRelease(IUnknown* Ptr)
{
    if (Ptr)
    {
        Ptr->Release();
    }
}

void DrawStateT::Bind(UBG_GfxContextT* Context, GfxSystem* System)
{
    Bind(Context, System, 0, nullptr, 0, nullptr);
}

void DrawStateT::Bind(UBG_GfxContextT* Context, GfxSystem* System, u32 InNumShaderRVs, ID3D11ShaderResourceView** ShaderRVs, u32 InNumSamplers, ID3D11SamplerState** Samplers)
{
    Context->IASetInputLayout(InputLayout);
    Context->VSSetShader(VertexShader, nullptr, 0);
    Context->PSSetShader(PixelShader, nullptr, 0);

    if (NumConstantBuffers)
    {
        ShaderBufferT ShaderBuffers[MaxConstantBuffers] = {};
        for (size_t Idx = 0; Idx < NumConstantBuffers; Idx++)
        {
            ShaderBufferT* pBuffer = System->ShaderBuffers.Get(idsConstantBuffers[Idx]);
            ASSERT(pBuffer);
            ShaderBuffers[Idx] = *pBuffer;
        }
        Context->VSSetConstantBuffers(0, NumConstantBuffers, ShaderBuffers);
        Context->PSSetConstantBuffers(0, NumConstantBuffers, ShaderBuffers);
    }

    ASSERT(NumShaderRVs == InNumShaderRVs && (InNumShaderRVs || !ShaderRVs));
    if (InNumShaderRVs)
    {
        Context->VSSetShaderResources(0, NumShaderRVs, ShaderRVs);
        Context->PSSetShaderResources(0, NumShaderRVs, ShaderRVs);
    }

    ASSERT(NumSamplers == InNumSamplers && (NumSamplers || !Samplers));
    if (InNumSamplers)
    {
        Context->VSSetSamplers(0, NumSamplers, Samplers);
        Context->PSSetSamplers(0, NumSamplers, Samplers);
    }
}

void DrawStateT::SafeRelease()
{
    ::SafeRelease(InputLayout);
    ::SafeRelease(VertexShader);
    ::SafeRelease(PixelShader);
}

void MeshStateT::Bind(UBG_GfxContextT* Context)
{
    u32 VxStride = (u32)VertexSize;
    u32 VxOffset = 0u;
    Context->IASetVertexBuffers(0, 1, &VxBuffer, &VxStride, &VxOffset);
    Context->IASetIndexBuffer(IxBuffer, DXGI_FORMAT_R32_UINT, 0);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void MeshStateT::Draw(UBG_GfxContextT* Context)
{
    if (IxBuffer)
    {
        Context->DrawIndexed((u32)NumInds, 0, 0);
    }
    else
    {
        Context->Draw((u32)NumVerts, 0);
    }
}

void MeshStateT::SafeRelease()
{
    ::SafeRelease(VxBuffer);
    ::SafeRelease(IxBuffer);
}

void GetClearColor(v4f& OutClearColor);
int CompileShaderHLSL
(
    FileContentsT* FileHLSL,
    const char* SourceFileName,
    LPCSTR EntryPointFunction,
    LPCSTR ShaderProfile,
    ID3DBlob** OutShaderBlob,
    const D3D_SHADER_MACRO* Defines
);
DrawStateT CreateDrawState
(
    ID3D11Device* Device,
    const char* ShaderFileName,
    const D3D_SHADER_MACRO* Defines,
    const D3D11_INPUT_ELEMENT_DESC* InputElements,
    size_t NumInputElements,
    u32 NumConstantBuffers,
    ShaderBufferID** idsConstantBuffers,
    u32 NumShaderRVs,
    u32 NumSamplers
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

void RenderEntitySystem::Init()
{
    Entities = {};
    Entities.Init();
}

void RenderEntitySystem::Term()
{
    Entities.Term();
}

RenderEntity* RenderEntitySystem::Get(RenderEntityID ID)
{
    return Entities.Get(ID);
}

RenderEntityID RenderEntitySystem::Create()
{
    return Entities.Create({});
}

RenderEntityID RenderEntitySystem::Create(RenderEntity InitData)
{
    return Entities.Create(InitData);
}

void RenderEntitySystem::Destroy(RenderEntityID ID)
{
    Entities.Destroy(ID);
}

void RenderEntitySystem::DrawAll(UBG_GfxContextT* Context, GfxSystem* System)
{
    ASSERT(System);
    ShaderBufferT* pViewProjBuffer = System->ShaderBuffers.Get(System->idViewProjBuffer);
    ASSERT(pViewProjBuffer);
    Context->UpdateSubresource(*pViewProjBuffer, 0, nullptr, &System->MainCameraO, (u32)sizeof(System->MainCameraO), 0);

    // TODO: We eventually want to distinguish between RenderEntities that share GPU data, and batch those calls together if possible
    for (size_t Idx = 0; Idx < Entities.NumActive; Idx++)
    {
        if (Entities.ActiveList[Idx].bVisible)
        {
            Entities.ActiveList[Idx].Draw(Context, System);
        }
    }
}

void RenderEntity::UpdateWorld(UBG_GfxContextT* Context, GfxSystem* System)
{
    ASSERT(System);
    ShaderBufferT* pWorldBuffer = System->ShaderBuffers.Get(System->idWorldBuffer);
    ASSERT(pWorldBuffer);
    Context->UpdateSubresource(*pWorldBuffer, 0, nullptr, &World, (u32)sizeof(m4f), 0);
}

void RenderEntity::Draw(UBG_GfxContextT* Context, GfxSystem* System)
{
    ASSERT(bVisible);

    MeshStateT* pMesh = System->Meshes.Get(idMesh);
    ASSERT(pMesh);

    switch (Type)
    {
        case DrawType::Color:
        {
            UpdateWorld(Context, System);
            pMesh->Bind(Context);
            DrawStateT* pDrawColor = System->DrawStates.Get(System->idsDrawState[(DrawStateID)DrawType::Color]);
            ASSERT(pDrawColor);
            pDrawColor->Bind(Context, System);
            pMesh->Draw(Context);
        } break;
        case DrawType::Texture:
        {
            UpdateWorld(Context, System);
            pMesh->Bind(Context);
            DrawStateT* pDrawTexture = System->DrawStates.Get(System->idsDrawState[(DrawStateID)DrawType::Texture]);
            ASSERT(pDrawTexture);
            TextureStateT* pTexture = System->Textures.Get(TextureState.idTexture);
            ASSERT(pTexture);
            SamplerStateT* pSampler = System->Samplers.Get(TextureState.idSampler);
            pDrawTexture->Bind(Context, System, 1u, &pTexture->SRV, 1u, pSampler);
            pMesh->Draw(Context);
        } break;
        case DrawType::Unicolor:
        {
            UpdateWorld(Context, System);
            ShaderBufferT* pUnicolorBuffer = System->ShaderBuffers.Get(System->idUnicolorBuffer);
            ASSERT(pUnicolorBuffer);
            DrawStateT* pDrawUnicolor = System->DrawStates.Get(System->idsDrawState[(DrawStateID)DrawType::Unicolor]);
            ASSERT(pDrawUnicolor)
            v4f UnicolorBufferData[4] = { UnicolorState.Color };
            Context->UpdateSubresource(*pUnicolorBuffer, 0, nullptr, UnicolorBufferData, (u32)sizeof(UnicolorBufferData), 0);
            pMesh->Bind(Context);
            pDrawUnicolor->Bind(Context, System);
            pMesh->Draw(Context);
        } break;
        default:
        {
            ASSERT(false);
        } break;
    }
}

void UBG_Gfx_DX11::DrawBegin()
{
    Context->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
    Context->OMSetBlendState(DefaultBlendState, nullptr, 0xFFFFFFFF);

    v4f ClearColor = { };
    GetClearColor(ClearColor);
    Context->ClearRenderTargetView(RenderTargetView, (float*)&ClearColor);
    Context->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void UBG_Gfx_DX11::DrawEnd()
{
    u32 SyncInterval = 0;
    u32 PresentFlags = 0;
    DXGI_PRESENT_PARAMETERS PresentParams = {};
    SwapChain->Present1(SyncInterval, PresentFlags, &PresentParams);
}

void UBG_Gfx_DX11::Draw()
{
    DrawBegin();
    GlobalEngine->Instance->Draw(Context);
    DrawEnd();
}

bool UBG_Gfx_DX11::Init()
{
    u32 CreateDeviceFlags = 0;
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
    SwapChainDesc1.Width = GlobalEngine->Width;
    SwapChainDesc1.Height = GlobalEngine->Height;
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
        GlobalEngine->PlatformState->hWindow,
        &SwapChainDesc1,
        nullptr,
        nullptr,
        &SwapChain
    );

    (void)SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);

    (void)Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderTargetView);

    D3D11_TEXTURE2D_DESC DepthDesc = {};
    DepthDesc.Width = GlobalEngine->Width;
    DepthDesc.Height = GlobalEngine->Height;
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
    ViewportDesc.Width = (FLOAT)GlobalEngine->Width;
    ViewportDesc.Height = (FLOAT)GlobalEngine->Height;
    ViewportDesc.MinDepth = 0.0f;
    ViewportDesc.MaxDepth = 1.0f;
    Context->RSSetViewports(1, &ViewportDesc);

    D3D11_RENDER_TARGET_BLEND_DESC RTBlendDesc = {};
    RTBlendDesc.BlendEnable = true;
    RTBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    RTBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    RTBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
    RTBlendDesc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    RTBlendDesc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    RTBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    RTBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    D3D11_BLEND_DESC BlendDesc = {};
    BlendDesc.AlphaToCoverageEnable = false;
    BlendDesc.IndependentBlendEnable = false;
    BlendDesc.RenderTarget[0] = RTBlendDesc;
    Device->CreateBlendState(&BlendDesc, &DefaultBlendState);

    Context->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);

    bool bResult = Device && Context &&
        SwapChain && BackBuffer &&
        RenderTargetView && RasterState &&
        DepthStencil && DepthStencilView &&
        DefaultBlendState;

    ASSERT(bResult);

    return bResult;
}

bool UBG_Gfx_DX11::Term()
{
    bool bResult = true;

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

    return bResult;
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
    FileContentsT* FileHLSL,
    const char* SourceFileName,
    LPCSTR EntryPointFunction,
    LPCSTR ShaderProfile,
    ID3DBlob** OutShaderBlob,
    const D3D_SHADER_MACRO* Defines
)
{
    if (FileHLSL == nullptr ||
        FileHLSL->Size == 0 ||
        FileHLSL->Contents == nullptr ||
        SourceFileName == nullptr ||
        EntryPointFunction == nullptr ||
        ShaderProfile == nullptr ||
        OutShaderBlob == nullptr)
    {
        return E_INVALIDARG;
    }

    HRESULT Result = S_OK;

    u32 ShaderCompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
    ShaderCompileFlags |= D3DCOMPILE_DEBUG;
    ShaderCompileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

    ID3DBlob* ShaderBlob = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    Result = D3DCompile2(
        FileHLSL->Contents,
        FileHLSL->Size,
        SourceFileName,
        Defines,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        EntryPointFunction,
        ShaderProfile,
        ShaderCompileFlags,
        0, // UINT Flags2
        0, // UINT SecondaryDataFlags
        nullptr, // LPCVOID pSecondaryData
        0, // SIZE_T SecondaryDataSize
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
    const char* ShaderFileName,
    const D3D_SHADER_MACRO* Defines,
    const D3D11_INPUT_ELEMENT_DESC* InputElements,
    size_t NumInputElements,
    u32 NumConstantBuffers,
    ShaderBufferID* idsConstantBuffers,
    u32 NumShaderRVs,
    u32 NumSamplers
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

    FileContentsT ShaderFile = {};
    ShaderFile.Load(ShaderFileName);
    ASSERT(ShaderFile.Size && ShaderFile.Contents);
    if (ShaderFile.Size && ShaderFile.Contents)
    {
        CompileShaderHLSL(&ShaderFile, ShaderFileName, VxShaderMain, VxShaderProfile, &VSBlob, Defines);
        CompileShaderHLSL(&ShaderFile, ShaderFileName, PxShaderMain, PxShaderProfile, &PSBlob, Defines);
        ShaderFile.Release();
    }

    DrawStateT Result = {};

    if (VSBlob && PSBlob)
    {
        Device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &Result.VertexShader);
        Device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &Result.PixelShader);

        Device->CreateInputLayout(InputElements, (u32)NumInputElements, VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &Result.InputLayout);
    }
    SafeRelease(VSBlob);
    SafeRelease(PSBlob);

    ASSERT(!NumConstantBuffers || idsConstantBuffers);
    if (NumConstantBuffers && idsConstantBuffers)
    {
        Result.NumConstantBuffers = NumConstantBuffers;
        for (u32 Idx = 0; Idx < NumConstantBuffers; Idx++)
        {
            Result.idsConstantBuffers[Idx] = idsConstantBuffers[Idx];
        }
    }
    Result.NumShaderRVs = NumShaderRVs;
    Result.NumSamplers = NumSamplers;

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
    D3D11_BUFFER_DESC VertexBufferDesc = { (u32)VxDataSize, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0 };
    D3D11_SUBRESOURCE_DATA VertexBufferInitData = { VertexData, 0, 0 };
    InDevice->CreateBuffer(&VertexBufferDesc, &VertexBufferInitData, &Result.VxBuffer);

    if (IndexData)
    {
        size_t IxDataSize = NumIndices * sizeof(unsigned int);
        D3D11_BUFFER_DESC IndexBufferDesc = { (u32)IxDataSize, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0 };
        D3D11_SUBRESOURCE_DATA IndexBufferInitData = { IndexData, 0, 0 };
        InDevice->CreateBuffer(&IndexBufferDesc, &IndexBufferInitData, &Result.IxBuffer);
    }

    return Result;
}

bool GfxSystem::Init(UBG_GfxT* _GfxBackend)
{
    ASSERT(_GfxBackend);
    GfxBackend = _GfxBackend;

    Entities = {};
    Entities.Init();

    DrawStates = {};
    DrawStates.Init();

    ShaderBuffers = {};
    ShaderBuffers.Init();

    Textures = {};
    Textures.Init();

    Samplers = {};
    Samplers.Init();

    Meshes = {};
    Meshes.Init();

    // Default texture / sampler:
    {
        ImageT DefaultImage = {};
        GetDebugImage(DefaultImage);
        idFallbackTexture = CreateTexture(&DefaultImage);
        delete[] DefaultImage.PxBuffer;

        D3D11_SAMPLER_DESC DefaultSamplerDesc = {};
        DefaultSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        D3D11_TEXTURE_ADDRESS_MODE Mode = D3D11_TEXTURE_ADDRESS_CLAMP; // D3D11_TEXTURE_ADDRESS_WRAP;
        DefaultSamplerDesc.AddressU = Mode;
        DefaultSamplerDesc.AddressV = Mode;
        DefaultSamplerDesc.AddressW = Mode;

        SamplerStateT DefaultSamplerState = {};
        GfxBackend->Device->CreateSamplerState(&DefaultSamplerDesc, &DefaultSamplerState);
        idDefaultSampler = Samplers.Create(DefaultSamplerState);
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

        ShaderBufferT WorldBuffer = {};
        GfxBackend->Device->CreateBuffer(&CommonBufferDesc, nullptr, &WorldBuffer);
        idWorldBuffer = ShaderBuffers.Create(WorldBuffer);

        ShaderBufferT ViewProjBuffer = {};
        CommonBufferDesc.ByteWidth = sizeof(m4f) * 2;
        GfxBackend->Device->CreateBuffer(&CommonBufferDesc, nullptr, &ViewProjBuffer);
        idViewProjBuffer = ShaderBuffers.Create(ViewProjBuffer);
    }

    ShaderBufferID idsWVPBuffers[] = { idWorldBuffer, idViewProjBuffer };

    // Shader color:
    {
        D3D11_INPUT_ELEMENT_DESC InputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        DrawStateT DrawColor = CreateDrawState
        (
            GfxBackend->Device,
            "src/hlsl/BaseShaderColor.hlsl",
            DefaultDefines,
            InputElements,
            ARRAY_SIZE(InputElements),
            ARRAY_SIZE(idsWVPBuffers), idsWVPBuffers,
            0, 0
        );
        idsDrawState[(size_t)DrawType::Color] = DrawStates.Create(DrawColor);
    }

    // Shader texture:
    {
        D3D11_INPUT_ELEMENT_DESC InputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        DrawStateT DrawTexture = CreateDrawState
        (
            GfxBackend->Device,
            "src/hlsl/BaseShaderTexture.hlsl",
            DefaultDefines,
            InputElements,
            ARRAY_SIZE(InputElements),
            ARRAY_SIZE(idsWVPBuffers), idsWVPBuffers,
            1, 1
        );
        idsDrawState[(size_t)DrawType::Texture] = DrawStates.Create(DrawTexture);
    }

    // Shader unicolor:
    {
        D3D11_BUFFER_DESC UnicolorBufferDesc = {};
        UnicolorBufferDesc.ByteWidth = sizeof(v4f);
        UnicolorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        UnicolorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        UnicolorBufferDesc.CPUAccessFlags = 0;
        ShaderBufferT UnicolorBuffer = {};
        GfxBackend->Device->CreateBuffer(&UnicolorBufferDesc, nullptr, &UnicolorBuffer);
        idUnicolorBuffer = ShaderBuffers.Create(UnicolorBuffer);

        ShaderBufferID idsCBuffers[] = { idsWVPBuffers[0], idsWVPBuffers[1], idUnicolorBuffer};

        D3D11_INPUT_ELEMENT_DESC InputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        DrawStateT DrawUnicolor = CreateDrawState
        (
            GfxBackend->Device,
            "src/hlsl/BaseShaderUnicolor.hlsl",
            DefaultDefines,
            InputElements,
            ARRAY_SIZE(InputElements),
            ARRAY_SIZE(idsCBuffers), idsCBuffers,
            0, 0
        );
        idsDrawState[(size_t)DrawType::Unicolor] = DrawStates.Create(DrawUnicolor);
    }

    // TODO: Why is Depth passed as -2?
    MainCameraO.Ortho((float)GlobalEngine->Width, (float)GlobalEngine->Height, -2.0f);

    // TODO: Check for correct init'd state here
    return true;
}

bool GfxSystem::Term()
{
    // TODO: Make sure we are actually clearing out gfx state / references correctly here
    Entities.Term();
    for (size_t Idx = 0; Idx < DrawStates.NumActive; Idx++)
    {
        DrawStates.ActiveList[Idx].SafeRelease();
    }
    DrawStates.Term();
    for (size_t Idx = 0; Idx < ShaderBuffers.NumActive; Idx++)
    {
        SafeRelease(ShaderBuffers.ActiveList[Idx]);
    }
    ShaderBuffers.Term();
    for (size_t Idx = 0; Idx < Textures.NumActive; Idx++)
    {
        SafeRelease(Textures.ActiveList[Idx].Texture);
        SafeRelease(Textures.ActiveList[Idx].SRV);
    }
    Textures.Term();
    for (size_t Idx = 0; Idx < Samplers.NumActive; Idx++)
    {
        SafeRelease(Samplers.ActiveList[Idx]);
    }
    Samplers.Term();
    for (size_t Idx = 0; Idx < Meshes.NumActive; Idx++)
    {
        Meshes.ActiveList[Idx].SafeRelease();
    }
    Meshes.Term();
    return true;
}

RenderEntityID GfxSystem::CreateEntity(RenderEntity EntityState)
{
    return Entities.Create(EntityState);
}

void GfxSystem::DestroyEntity(RenderEntityID ID)
{
    Entities.Destroy(ID);
}

MeshStateID GfxSystem::CreateMesh(size_t VertexSize, size_t NumVertices, void* VertexData, size_t NumIndices, u32* IndexData)
{
    MeshStateT NewMesh = CreateMeshState
    (
        GfxBackend->Device,
        VertexSize,
        NumVertices,
        VertexData,
        NumIndices,
        IndexData
    );
    return Meshes.Create(NewMesh);
}

void GfxSystem::DestroyMesh(MeshStateID ID)
{
    Meshes.Destroy(ID);
}

TextureStateID GfxSystem::CreateTexture(ImageT* Image)
{
    ASSERT(Image);

    D3D11_SUBRESOURCE_DATA TextureResDataDesc[] = { {} };
    TextureResDataDesc[0].pSysMem = Image->PxBuffer;
    TextureResDataDesc[0].SysMemPitch = sizeof(RGBA32) * Image->Width;
    TextureResDataDesc[0].SysMemSlicePitch = TextureResDataDesc[0].SysMemPitch * Image->Height;
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = Image->Width;
    TextureDesc.Height = Image->Height;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    TextureDesc.SampleDesc = { 1, 0 };
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    TextureStateT NewTexture = {};
    GfxBackend->Device->CreateTexture2D(&TextureDesc, &TextureResDataDesc[0], &NewTexture.Texture);
    GfxBackend->Device->CreateShaderResourceView(NewTexture.Texture, nullptr, &NewTexture.SRV);
    return Textures.Create(NewTexture);
}

void GfxSystem::DestroyTexture(TextureStateID ID)
{
    Textures.Destroy(ID);
}



