#include "UBG.h" // E UBG_Gfx.h E UBG_GfxDX11.h

static_assert(UBG_GFX_IMPL_DX11(), "UBG_GfxDX11.cpp: File being built without DX11 GfxBackend impl defined");

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


void DrawStateT::Bind(GfxSystem* System)
{
    Bind(System, 0, nullptr, 0, nullptr);
}

void DrawStateT::Bind(GfxSystem* System, u32 InNumShaderRVs, ID3D11ShaderResourceView** ShaderRVs, u32 InNumSamplers, ID3D11SamplerState** Samplers)
{
    ASSERT(System && System->Backend && System->Backend->Context);
    UBG_GfxContextT* Context = System->Backend->Context;

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

void MeshInstStateT::Bind(UBG_GfxContextT* Context)
{
    ID3D11Buffer* VxInstBuffers[] = { VxBuffer, InstBuffer };
    u32 VxInstStrides[] = { (u32)VertexSize, (u32)PerInstSize };
    u32 VxInstOffsets[] = { 0, 0 };
    Context->IASetVertexBuffers(0, ARRAY_SIZE(VxInstBuffers), VxInstBuffers, VxInstStrides, VxInstOffsets);
    Context->IASetIndexBuffer(IxBuffer, DXGI_FORMAT_R32_UINT, 0);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void MeshInstStateT::Draw(UBG_GfxContextT* Context, size_t NumInst, void* pInstData)
{
    ASSERT(NumInst && pInstData);

    u32 NumInstPerCall = (u32)MaxInstCount;
    u8* InstReadPtr = (u8*)pInstData;
    u32 NumRemainingInst = (u32)NumInst;
    while (NumRemainingInst > 0)
    {
        u32 CurrDrawNumInst = Min(NumRemainingInst, NumInstPerCall);
        // Map instance data
        {
            D3D11_MAPPED_SUBRESOURCE InstDataMapWrite = {};
            ASSERT(!FAILED(Context->Map(InstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &InstDataMapWrite)));
            memcpy(InstDataMapWrite.pData, InstReadPtr, CurrDrawNumInst * PerInstSize);
            Context->Unmap(InstBuffer, 0);
        }

        // Draw instances
        if (IxBuffer) { Context->DrawIndexedInstanced((u32)NumInds, (u32)CurrDrawNumInst, 0, 0, 0); }
        else { Context->DrawInstanced((u32)NumVerts, (u32)CurrDrawNumInst, 0, 0); }

        InstReadPtr += CurrDrawNumInst * PerInstSize;
        NumRemainingInst -= CurrDrawNumInst;
    }
}

void MeshInstStateT::SafeRelease()
{
    ::SafeRelease(VxBuffer);
    ::SafeRelease(InstBuffer);
    ::SafeRelease(IxBuffer);
}

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
MeshInstStateT CreateMeshInstState
(
    ID3D11Device* InDevice,
    size_t VertexSize,
    size_t PerInstSize,
    size_t MaxInstCount,
    size_t NumVertices,
    void* VertexData,
    size_t NumIndices,
    u32* IndexData
);

RenderEntity RenderEntity::Default(m4f World, DrawType Type, MeshStateID idMesh)
{
    RenderEntity Result = {};
    Result.bVisible = true;
    Result.World = World;
    Result.Type = Type;
    Result.idMesh = idMesh;
    switch (Type)
    {
        case DrawType::Color: { Result.ColorState = {}; } break;
        case DrawType::Texture: { Result.TextureState = {}; } break;
        case DrawType::Unicolor: { Result.UnicolorState = {}; } break;
        default: { ASSERT(false); } break;
    }
    return Result;
}

void RenderEntitySystem::Init()
{
    Entities = {};
    Entities.Init();
    EntitiesInst = {};
    EntitiesInst.Init();
}

void RenderEntitySystem::Term()
{
    Entities.Term();
    EntitiesInst.Term();
}

RenderEntity* RenderEntitySystem::Get(RenderEntityID ID)
{
    return Entities.Get(ID);
}

RenderInstEntity* RenderEntitySystem::GetInst(RenderInstEntityID ID)
{
    return EntitiesInst.Get(ID);
}

RenderEntityID RenderEntitySystem::Create()
{
    return Entities.Create({});
}

RenderInstEntityID RenderEntitySystem::CreateInst()
{
    return EntitiesInst.Create({});
}

RenderEntityID RenderEntitySystem::Create(RenderEntity InitData)
{
    return Entities.Create(InitData);
}

RenderInstEntityID RenderEntitySystem::CreateInst(RenderInstEntity InitData)
{
    return EntitiesInst.Create(InitData);
}

void RenderEntitySystem::Destroy(RenderEntityID ID)
{
    Entities.Destroy(ID);
}

void RenderEntitySystem::DestroyInst(RenderInstEntityID ID)
{
    EntitiesInst.Destroy(ID);
}

void RenderEntitySystem::DrawAll(GfxSystem* System)
{
    ASSERT(System && System->Backend && System->Backend->Context);
    UBG_GfxContextT* Context = System->Backend->Context;
    ShaderBufferT* pViewProjBuffer = System->ShaderBuffers.Get(System->idViewProjBuffer);
    ASSERT(pViewProjBuffer);
    Context->UpdateSubresource(*pViewProjBuffer, 0, nullptr, &System->MainCameraO, (u32)sizeof(System->MainCameraO), 0);

    // TODO: We eventually want to distinguish between RenderEntities that share GPU data, and batch those calls together if possible
    for (size_t Idx = 0; Idx < Entities.NumActive; Idx++)
    {
        if (Entities.ActiveList[Idx].bVisible)
        {
            Entities.ActiveList[Idx].Draw(System);
        }
    }

    for (size_t Idx = 0; Idx < EntitiesInst.NumActive; Idx++)
    {
        if (EntitiesInst.ActiveList[Idx].bVisible)
        {
            EntitiesInst.ActiveList[Idx].Draw(System);
        }
    }
}

void RenderEntity::UpdateWorld(GfxSystem* System)
{
    ASSERT(System && System->Backend && System->Backend->Context);
    UBG_GfxContextT* Context = System->Backend->Context;
    ShaderBufferT* pWorldBuffer = System->ShaderBuffers.Get(System->idWorldBuffer);
    ASSERT(pWorldBuffer);
    Context->UpdateSubresource(*pWorldBuffer, 0, nullptr, &World, (u32)sizeof(m4f), 0);
}

void RenderEntity::Draw(GfxSystem* System)
{
    ASSERT(bVisible);
    ASSERT(System && System->Backend && System->Backend->Context);
    UBG_GfxContextT* Context = System->Backend->Context;
    MeshStateT* pMesh = System->Meshes.Get(idMesh);
    ASSERT(pMesh);

    switch (Type)
    {
        case DrawType::Color:
        {
            UpdateWorld(System);
            pMesh->Bind(Context);
            DrawStateT* pDrawColor = System->DrawStates.Get(System->idsDrawState[(DrawStateID)DrawType::Color]);
            ASSERT(pDrawColor);
            pDrawColor->Bind(System);
            pMesh->Draw(Context);
        } break;
        case DrawType::Texture:
        {
            UpdateWorld(System);
            pMesh->Bind(Context);
            DrawStateT* pDrawTexture = System->DrawStates.Get(System->idsDrawState[(DrawStateID)DrawType::Texture]);
            ASSERT(pDrawTexture);
            TextureStateT* pTexture = System->Textures.Get(TextureState.idTexture);
            ASSERT(pTexture);
            SamplerStateT* pSampler = System->Samplers.Get(TextureState.idSampler);
            pDrawTexture->Bind(System, 1u, &pTexture->SRV, 1u, pSampler);
            pMesh->Draw(Context);
        } break;
        case DrawType::Unicolor:
        {
            UpdateWorld(System);
            ShaderBufferT* pUnicolorBuffer = System->ShaderBuffers.Get(System->idUnicolorBuffer);
            ASSERT(pUnicolorBuffer);
            DrawStateT* pDrawUnicolor = System->DrawStates.Get(System->idsDrawState[(DrawStateID)DrawType::Unicolor]);
            ASSERT(pDrawUnicolor)
            v4f UnicolorBufferData[4] = { UnicolorState.Color };
            Context->UpdateSubresource(*pUnicolorBuffer, 0, nullptr, UnicolorBufferData, (u32)sizeof(UnicolorBufferData), 0);
            pMesh->Bind(Context);
            pDrawUnicolor->Bind(System);
            pMesh->Draw(Context);
        } break;
        default:
        {
            ASSERT(false);
        } break;
    }
}

void RenderInstEntity::UpdateWorld(GfxSystem* System)
{
    ASSERT(System && System->Backend && System->Backend->Context);
    UBG_GfxContextT* Context = System->Backend->Context;
    ShaderBufferT* pWorldBuffer = System->ShaderBuffers.Get(System->idWorldBuffer);
    ASSERT(pWorldBuffer);
    Context->UpdateSubresource(*pWorldBuffer, 0, nullptr, &World, (u32)sizeof(m4f), 0);
}

void RenderInstEntity::Draw(GfxSystem* System)
{
    ASSERT(bVisible);
    ASSERT(System && System->Backend && System->Backend->Context);

    // Early return if nothing to draw
    if (!NumInst || !pInstData) { return; }
    ASSERT(NumInst);
    ASSERT(pInstData);

    UBG_GfxContextT* Context = System->Backend->Context;
    MeshInstStateT* pMesh = System->MeshesInst.Get(idMesh);
    ASSERT(pMesh);

    switch (Type)
    {
        case DrawInstType::Color:
        {
            UpdateWorld(System);
            pMesh->Bind(Context);
            DrawStateT* pDrawInstColor = System->DrawInstStates.Get(System->idsDrawInstState[(size_t)DrawInstType::Color]);
            ASSERT(pDrawInstColor);
            pDrawInstColor->Bind(System);
            pMesh->Draw(Context, NumInst, pInstData);

        } break;

        default:
        {
            ASSERT(false);
        } break;
    }

    // NOTE: For safety reasons, we clear out the NumInst and pInstData after drawing
    //      This requires the owner of this RenderInstEntity to explicitly set both each frame
    NumInst = 0;
    pInstData = nullptr;
}

void UBG_Gfx_DX11::DrawBegin()
{
    Context->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
    Context->OMSetBlendState(DefaultBlendState, nullptr, 0xFFFFFFFF);

    v4f ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    static constexpr bool bEnableSeizureDebug = true;
    if (bEnableSeizureDebug)
    {
        constexpr v4f Black{ 0.0f, 0.0f, 0.0f, 1.0f };
        constexpr v4f Pink{ 1.0f, 73.0f / 255.0f, 173.0f / 255.0f, 1.0f };
        constexpr float StepDurationSeconds = 1.0f;

        static float LastSwitchTime = 0.0f;
        static bool bBlack = true;
        float CurrTime = (float)GlobalEngine->Clock->CurrTime;
        if ((CurrTime - LastSwitchTime) > StepDurationSeconds)
        {
            LastSwitchTime = CurrTime;
            bBlack = !bBlack;
        }
        ClearColor = bBlack ? Black : Pink;
    }

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
    GlobalEngine->Instance->Draw();
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

MeshInstStateT CreateMeshInstState
(
    ID3D11Device* InDevice,
    size_t VertexSize,
    size_t PerInstSize,
    size_t MaxInstCount,
    size_t NumVertices,
    void* VertexData,
    size_t NumIndices,
    u32* IndexData
)
{
    MeshInstStateT Result;

    Result.VertexSize = VertexSize;
    Result.NumVerts = NumVertices;
    Result.NumInds = NumIndices;
    Result.PerInstSize = PerInstSize;
    Result.MaxInstCount = MaxInstCount;
    Result.InstBufferSize = PerInstSize * MaxInstCount;

    size_t VxDataSize = VertexSize * NumVertices;
    D3D11_BUFFER_DESC VertexBufferDesc = { (u32)VxDataSize, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0 };
    D3D11_SUBRESOURCE_DATA VertexBufferInitData = { VertexData, 0, 0 };
    InDevice->CreateBuffer(&VertexBufferDesc, &VertexBufferInitData, &Result.VxBuffer);

    D3D11_BUFFER_DESC InstBufferDesc = { (u32)Result.InstBufferSize, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, 0 };
    InDevice->CreateBuffer(&InstBufferDesc, nullptr, &Result.InstBuffer);

    if (IndexData)
    {
        size_t IxDataSize = NumIndices * sizeof(u32);
        D3D11_BUFFER_DESC IndexBufferDesc = { (u32)IxDataSize, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0 };
        D3D11_SUBRESOURCE_DATA IndexBufferInitData = { IndexData, 0, 0 };
        InDevice->CreateBuffer(&IndexBufferDesc, &IndexBufferInitData, &Result.IxBuffer);
    }

    return Result;
}

bool GfxSystem::Init(UBG_GfxT* _GfxBackend)
{
    ASSERT(_GfxBackend);
    Backend = _GfxBackend;

    Entities = {};
    Entities.Init();

    DrawStates = {};
    DrawStates.Init();

    DrawInstStates = {};
    DrawInstStates.Init();

    ShaderBuffers = {};
    ShaderBuffers.Init();

    Textures = {};
    Textures.Init();

    Samplers = {};
    Samplers.Init();

    Meshes = {};
    Meshes.Init();

    MeshesInst = {};
    MeshesInst.Init();
    // TODO: The above sucks! having to do all of these calls sucks
    //      We should at least remove the = {} by having inline default values in each struct definition

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
        Backend->Device->CreateSamplerState(&DefaultSamplerDesc, &DefaultSamplerState);
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
        Backend->Device->CreateBuffer(&CommonBufferDesc, nullptr, &WorldBuffer);
        idWorldBuffer = ShaderBuffers.Create(WorldBuffer);

        ShaderBufferT ViewProjBuffer = {};
        CommonBufferDesc.ByteWidth = sizeof(m4f) * 2;
        Backend->Device->CreateBuffer(&CommonBufferDesc, nullptr, &ViewProjBuffer);
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
            Backend->Device,
            "src/hlsl/BaseShaderColor.hlsl",
            DefaultDefines,
            InputElements,
            ARRAY_SIZE(InputElements),
            ARRAY_SIZE(idsWVPBuffers), idsWVPBuffers,
            0, 0
        );
        idsDrawState[(size_t)DrawType::Color] = DrawStates.Create(DrawColor);
        ASSERT(idsDrawState[(size_t)DrawType::Color]);
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
            Backend->Device,
            "src/hlsl/BaseShaderTexture.hlsl",
            DefaultDefines,
            InputElements,
            ARRAY_SIZE(InputElements),
            ARRAY_SIZE(idsWVPBuffers), idsWVPBuffers,
            1, 1
        );
        idsDrawState[(size_t)DrawType::Texture] = DrawStates.Create(DrawTexture);
        ASSERT(idsDrawState[(size_t)DrawType::Texture]);
    }

    // Shader unicolor:
    {
        D3D11_BUFFER_DESC UnicolorBufferDesc = {};
        UnicolorBufferDesc.ByteWidth = sizeof(v4f);
        UnicolorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        UnicolorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        UnicolorBufferDesc.CPUAccessFlags = 0;
        ShaderBufferT UnicolorBuffer = {};
        Backend->Device->CreateBuffer(&UnicolorBufferDesc, nullptr, &UnicolorBuffer);
        idUnicolorBuffer = ShaderBuffers.Create(UnicolorBuffer);

        ShaderBufferID idsCBuffers[] = { idsWVPBuffers[0], idsWVPBuffers[1], idUnicolorBuffer};

        D3D11_INPUT_ELEMENT_DESC InputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        DrawStateT DrawUnicolor = CreateDrawState
        (
            Backend->Device,
            "src/hlsl/BaseShaderUnicolor.hlsl",
            DefaultDefines,
            InputElements,
            ARRAY_SIZE(InputElements),
            ARRAY_SIZE(idsCBuffers), idsCBuffers,
            0, 0
        );
        idsDrawState[(size_t)DrawType::Unicolor] = DrawStates.Create(DrawUnicolor);
        ASSERT(idsDrawState[(size_t)DrawType::Unicolor]);
    }

    // Inst shader color:
    {
        D3D11_INPUT_ELEMENT_DESC InputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "RECT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        };
        DrawStateT DrawInstColor = CreateDrawState
        (
            Backend->Device,
            "src/hlsl/BaseInstShader.hlsl",
            DefaultDefines,
            InputElements,
            ARRAY_SIZE(InputElements),
            ARRAY_SIZE(idsWVPBuffers), idsWVPBuffers,
            0, 0
        );
        idsDrawInstState[(size_t)DrawInstType::Color] = DrawInstStates.Create(DrawInstColor);
        ASSERT(idsDrawInstState[(size_t)DrawInstType::Color]);
    }

    // Platonic quad:
    {
        VxTex QuadVertsTexture[] = {
            { { -0.5f, +0.5f, +0.5f, 1.0f}, { 0.0f, 0.0f } },
            { { +0.5f, +0.5f, +0.5f, 1.0f}, { 1.0f, 0.0f } },
            { { -0.5f, -0.5f, +0.5f, 1.0f}, { 0.0f, 1.0f } },
            { { +0.5f, -0.5f, +0.5f, 1.0f}, { 1.0f, 1.0f } },
        };

        u32 QuadInds[] = { 0, 1, 2,    1, 3, 2 };

        idQuadTexture = CreateMesh(
            sizeof(VxTex),
            ARRAY_SIZE(QuadVertsTexture), QuadVertsTexture,
            ARRAY_SIZE(QuadInds), QuadInds
        );
        ASSERT(idQuadTexture);

        /*
        MeshStateID idQuadUnicolorOrigin;

        VxMin QuadVertsUnicolorOrigin[] = {
            { { -0.5f, +0.5f, +0.5f, 1.0f} },
            { { +0.5f, +0.5f, +0.5f, 1.0f} },
            { { -0.5f, -0.5f, +0.5f, 1.0f} },
            { { +0.5f, -0.5f, +0.5f, 1.0f} },
        };

        idQuadUnicolorOrigin = CreateMesh(
            sizeof(VxMin),
            ARRAY_SIZE(QuadVertsUnicolorOrigin), QuadVertsUnicolorOrigin,
            ARRAY_SIZE(QuadInds), QuadInds
        );
        ASSERT(idQuadUnicolorOrigin);
        */

        VxMin QuadVertsUnicolor[] = {
            { { 0.0f, 1.0f, +0.5f, 1.0f} },
            { { 1.0f, 1.0f, +0.5f, 1.0f} },
            { { 0.0f, 0.0f, +0.5f, 1.0f} },
            { { 1.0f, 0.0f, +0.5f, 1.0f} },
        };
        idQuadUnicolor = CreateMesh(
            sizeof(VxMin),
            ARRAY_SIZE(QuadVertsUnicolor), QuadVertsUnicolor,
            ARRAY_SIZE(QuadInds), QuadInds
        );
        ASSERT(idQuadUnicolor);

        idInstRectColor = CreateMeshInst(
            sizeof(VxMin),
            sizeof(InstRectColorData),
            MeshInstStateT::DefaultMaxInstCount,
            ARRAY_SIZE(QuadVertsUnicolor),
            QuadVertsUnicolor,
            ARRAY_SIZE(QuadInds),
            QuadInds
        );
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

RenderEntity* GfxSystem::GetEntity(RenderEntityID ID)
{
    return Entities.Get(ID);
}

RenderInstEntity* GfxSystem::GetEntityInst(RenderInstEntityID ID)
{
    return Entities.GetInst(ID);
}

RenderEntityID GfxSystem::CreateEntity(RenderEntity EntityState)
{
    return Entities.Create(EntityState);
}

RenderInstEntityID GfxSystem::CreateEntityInst(RenderInstEntity EntityState)
{
    return Entities.CreateInst(EntityState);
}

void GfxSystem::DestroyEntity(RenderEntityID ID)
{
    Entities.Destroy(ID);
}

void GfxSystem::DestroyEntityInst(RenderInstEntityID ID)
{
    Entities.DestroyInst(ID);
}

MeshStateT* GfxSystem::GetMesh(MeshStateID ID)
{
    return Meshes.Get(ID);
}

MeshInstStateT* GfxSystem::GetMeshInst(MeshInstStateID ID)
{
    return MeshesInst.Get(ID);
}

MeshStateID GfxSystem::CreateMesh(size_t VertexSize, size_t NumVertices, void* VertexData, size_t NumIndices, u32* IndexData)
{
    MeshStateT NewMesh = CreateMeshState
    (
        Backend->Device,
        VertexSize,
        NumVertices,
        VertexData,
        NumIndices,
        IndexData
    );
    return Meshes.Create(NewMesh);
}

MeshInstStateID GfxSystem::CreateMeshInst(size_t VertexSize, size_t PerInstSize, size_t MaxInstCount, size_t NumVertices, void* VertexData, size_t NumIndices, u32* IndexData)
{
    MeshInstStateT NewMesh = CreateMeshInstState
    (
        Backend->Device,
        VertexSize,
        PerInstSize,
        MaxInstCount,
        NumVertices,
        VertexData,
        NumIndices,
        IndexData
    );
    return MeshesInst.Create(NewMesh);
}

void GfxSystem::DestroyMesh(MeshStateID ID)
{
    Meshes.Destroy(ID);
}

void GfxSystem::DestroyMeshInst(MeshInstStateID ID)
{
    MeshesInst.Destroy(ID);
}

TextureStateT* GfxSystem::GetTexture(TextureStateID ID)
{
    return Textures.Get(ID);
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
    Backend->Device->CreateTexture2D(&TextureDesc, &TextureResDataDesc[0], &NewTexture.Texture);
    Backend->Device->CreateShaderResourceView(NewTexture.Texture, nullptr, &NewTexture.SRV);
    return Textures.Create(NewTexture);
}

void GfxSystem::DestroyTexture(TextureStateID ID)
{
    Textures.Destroy(ID);
}



