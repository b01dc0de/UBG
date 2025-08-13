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

    static constexpr u32 MaxConstantBuffers = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
    static constexpr u32 MaxShaderRVs = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
    static constexpr u32 MaxSamplers = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
    u32 NumConstantBuffers;
    u32 NumShaderRVs;
    u32 NumSamplers;
    ID3D11Buffer* ConstantBuffers[MaxConstantBuffers];
    ID3D11ShaderResourceView* ShaderRVs[MaxShaderRVs];
    ID3D11SamplerState* Samplers[MaxSamplers];

    void Bind(ID3D11DeviceContext* Context)
    {
        Context->IASetInputLayout(InputLayout);
        Context->VSSetShader(VertexShader, nullptr, 0);
        Context->PSSetShader(PixelShader, nullptr, 0);

        if (NumConstantBuffers)
        {
            Context->VSSetConstantBuffers(0, NumConstantBuffers, ConstantBuffers);
            Context->PSSetConstantBuffers(0, NumConstantBuffers, ConstantBuffers);
        }
        if (NumShaderRVs)
        {
            Context->VSSetShaderResources(0, NumShaderRVs, ShaderRVs);
            Context->PSSetShaderResources(0, NumShaderRVs, ShaderRVs);
        }
        if (NumSamplers)
        {
            Context->VSSetSamplers(0, NumSamplers, Samplers);
            Context->PSSetSamplers(0, NumSamplers, Samplers);
        }
    }

    void SafeRelease()
    {
        ::SafeRelease(InputLayout);
        ::SafeRelease(VertexShader);
        ::SafeRelease(PixelShader);
    }
};

void MeshStateT::Bind(ID3D11DeviceContext* Context)
{
    u32 VxStride = (u32)VertexSize;
    u32 VxOffset = 0u;
    Context->IASetVertexBuffers(0, 1, &VxBuffer, &VxStride, &VxOffset);
    Context->IASetIndexBuffer(IxBuffer, DXGI_FORMAT_R32_UINT, 0);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void MeshStateT::Draw(ID3D11DeviceContext* Context)
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

struct GfxImpl
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

    static RenderEntitySystem RE_System;
    static RenderEntityID RE_ColorTriangle;
    static RenderEntityID RE_QuadTexture;
    static RenderEntityID RE_QuadUnicolor;

    static void Draw(ID3D11DeviceContext* Context);
    static void DrawDemo(ID3D11DeviceContext* Context);
    static void DrawGame(ID3D11DeviceContext* Context);
    static bool Init(ID3D11Device* Device);
    static bool Term();
};

ID3D11Texture2D* GfxImpl::DefaultTexture = {};
ID3D11ShaderResourceView* GfxImpl::DefaultTextureSRV = {};
ID3D11SamplerState* GfxImpl::DefaultSamplerState = {};
ID3D11Buffer* GfxImpl::WorldBuffer = {};
ID3D11Buffer* GfxImpl::ViewProjBuffer = {};

DrawStateT GfxImpl::DrawColor = {};
DrawStateT GfxImpl::DrawTexture = {};
DrawStateT GfxImpl::DrawUnicolor = {};
ID3D11Buffer* GfxImpl::UnicolorBuffer = {};
MeshStateT GfxImpl::MeshTriangle = {};
MeshStateT GfxImpl::MeshQuad = {};
MeshStateT GfxImpl::MeshQuadMin = {};

Camera GfxImpl::CameraO = {};

RenderEntitySystem GfxImpl::RE_System = {0};
RenderEntityID GfxImpl::RE_ColorTriangle = {};
RenderEntityID GfxImpl::RE_QuadTexture = {};
RenderEntityID GfxImpl::RE_QuadUnicolor = {};

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
    size_t NumInputElements,
    u32 NumConstantBuffers,
    ID3D11Buffer** ConstantBuffers,
    u32 NumShaderRVs,
    ID3D11ShaderResourceView** ShaderRVs,
    u32 NumSamplers,
    ID3D11SamplerState** Samplers
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
    Entities.Reserve(MaxEntities);
    CounterID = 0;
}

void RenderEntitySystem::Term()
{
    Entities.Term();
}

RenderEntity* RenderEntitySystem::Get(RenderEntityID ID)
{
    // TODO: Actually make this a fast ID lookup, not just searching through
    RenderEntity* Result = nullptr;
    for (size_t Idx = 0; Idx < Entities.Num; Idx++)
    {
        if (Entities[Idx].ID == ID)
        {
            Result = &Entities[Idx];
            break;
        }
    }
    return Result;
}

RenderEntityID RenderEntitySystem::Create()
{
    ASSERT(Entities.Num < MaxEntities);
    RenderEntityID NewID = CounterID++;
    RenderEntity NewEntity = {};
    NewEntity.ID = NewID;
    Entities.Add(NewEntity);
    return NewID;
}

void RenderEntitySystem::Destroy(RenderEntityID ID)
{
    RenderEntity* ToDestroy = Get(ID);
    ASSERT(ToDestroy);
    if (ToDestroy)
    {
        size_t IdxToRemove = ToDestroy - Entities.Data;
        ASSERT(IdxToRemove < Entities.Num);
        if (IdxToRemove < Entities.Num)
        {
            Entities.RemoveQ(IdxToRemove);
        }
    }
}

void RenderEntitySystem::DrawAll(ID3D11DeviceContext* Context)
{
    Context->UpdateSubresource(GfxImpl::ViewProjBuffer, 0, nullptr, &GfxImpl::CameraO, (u32)sizeof(GfxImpl::CameraO), 0);

    for (size_t Idx = 0; Idx < Entities.Num; Idx++)
    {
        if (Entities[Idx].bVisible)
        {
            Entities[Idx].Draw(Context);
        }
    }
}

void RenderEntity::UpdateWorld(ID3D11DeviceContext* Context)
{
    Context->UpdateSubresource(GfxImpl::WorldBuffer, 0, nullptr, &World, (u32)sizeof(m4f), 0);
}

void RenderEntity::Draw(ID3D11DeviceContext* Context)
{
    ASSERT(bVisible);

    switch (Type)
    {
        case DrawType::Color:
        {
            UpdateWorld(Context);
            Mesh->Bind(Context);
            GfxImpl::DrawColor.Bind(Context);
            Mesh->Draw(Context);
        } break;
        case DrawType::Texture:
        {
            UpdateWorld(Context);
            Mesh->Bind(Context);
            //GfxImpl::DrawTexture.Bind(Context);
            {
                Context->IASetInputLayout(GfxImpl::DrawTexture.InputLayout);
                Context->VSSetShader(GfxImpl::DrawTexture.VertexShader, nullptr, 0);
                Context->PSSetShader(GfxImpl::DrawTexture.PixelShader, nullptr, 0);
                if (GfxImpl::DrawTexture.NumConstantBuffers)
                {
                    Context->VSSetConstantBuffers(0, GfxImpl::DrawTexture.NumConstantBuffers, GfxImpl::DrawTexture.ConstantBuffers);
                    Context->PSSetConstantBuffers(0, GfxImpl::DrawTexture.NumConstantBuffers, GfxImpl::DrawTexture.ConstantBuffers);
                }
                Context->VSSetShaderResources(0, 1, &TextureState.TextureSRV);
                Context->PSSetShaderResources(0, 1, &TextureState.TextureSRV);
                Context->VSSetSamplers(0, 1, &TextureState.Sampler);
                Context->PSSetSamplers(0, 1, &TextureState.Sampler);
            }
            Mesh->Draw(Context);
        } break;
        case DrawType::Unicolor:
        {
            UpdateWorld(Context);
            Context->UpdateSubresource(GfxImpl::WorldBuffer, 0, nullptr, &World, (u32)sizeof(m4f), 0);
            v4f UnicolorBufferData[4] = { UnicolorState.Color };
            Context->UpdateSubresource(GfxImpl::UnicolorBuffer, 0, nullptr, UnicolorBufferData, (u32)sizeof(UnicolorBufferData), 0);
            Mesh->Bind(Context);
            GfxImpl::DrawUnicolor.Bind(Context);
            Mesh->Draw(Context);
        } break;
        default:
        {
            ASSERT(false);
        } break;
    }
}


void GfxImpl::Draw(ID3D11DeviceContext* Context)
{
    constexpr bool bDrawDemo = false;

    if (bDrawDemo)
    {
        DrawDemo(Context);
    }
    else
    {
        DrawGame(Context);
    }
}

void GfxImpl::DrawDemo(ID3D11DeviceContext* Context)
{
    float HalfWidth = GlobalState::Width * 0.5f;
    float HalfHeight = GlobalState::Height * 0.5f;
    m4f SpriteWorld = m4f::Scale(HalfWidth, HalfHeight, 1.0f) * m4f::Trans(0.0f, 0.0f, 0.0f);
    Context->UpdateSubresource(WorldBuffer, 0, nullptr, &SpriteWorld, (u32)sizeof(m4f), 0);
    Context->UpdateSubresource(ViewProjBuffer, 0, nullptr, &CameraO, (u32)sizeof(CameraO), 0);

    // MeshTriangle / DrawColor / DrawColorShaderState:
    {
        MeshTriangle.Bind(Context);
        DrawColor.Bind(Context);
        MeshTriangle.Draw(Context);
    }

    // MeshQuad / DrawTexture / DrawTextureShaderState:
    {
        MeshQuad.Bind(Context);
        DrawTexture.Bind(Context);
        MeshQuad.Draw(Context);
    }

    // MeshQuadMin / DrawUnicolor / DrawUnicolorShaderState:
    {
        v4f UnicolorData[4] = { };
        GetClearColor(UnicolorData[0]);
        // Set it to opposite color of clear color for now
        UnicolorData[0] = { 1.0f - UnicolorData[0].X, 1.0f - UnicolorData[0].Y, 1.0f - UnicolorData[0].Z, 1.0f };
        Context->UpdateSubresource(UnicolorBuffer, 0, nullptr, UnicolorData, (u32)sizeof(UnicolorData), 0);

        MeshQuadMin.Bind(Context);
        DrawUnicolor.Bind(Context);
        MeshQuadMin.Draw(Context);
    }
}

void GfxImpl::DrawGame(ID3D11DeviceContext* Context)
{
    RE_System.DrawAll(Context);
}

bool GfxImpl::Init(ID3D11Device* Device)
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

    ID3D11Buffer* WVPBuffers[] = { WorldBuffer, ViewProjBuffer };

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
            ARRAY_SIZE(InputElements),
            ARRAY_SIZE(WVPBuffers), WVPBuffers,
            0, nullptr,
            0, nullptr
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
            ARRAY_SIZE(InputElements),
            ARRAY_SIZE(WVPBuffers), WVPBuffers,
            1, &DefaultTextureSRV,
            1, &DefaultSamplerState
        );
    }

    // Shader unicolor:
    {
        D3D11_BUFFER_DESC UnicolorBufferDesc = {};
        UnicolorBufferDesc.ByteWidth = sizeof(v4f);
        UnicolorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        UnicolorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        UnicolorBufferDesc.CPUAccessFlags = 0;
        Device->CreateBuffer(&UnicolorBufferDesc, nullptr, &UnicolorBuffer);

        ID3D11Buffer* CBuffers[] = { WorldBuffer, ViewProjBuffer, UnicolorBuffer };

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
            ARRAY_SIZE(InputElements),
            ARRAY_SIZE(CBuffers), CBuffers,
            0, nullptr,
            0, nullptr
        );
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

    RE_System.Init();
    RE_ColorTriangle = RE_System.Create();
    RE_QuadTexture = RE_System.Create();
    RE_QuadUnicolor = RE_System.Create();

    RenderEntity* ColorTriangleData = RE_System.Get(RE_ColorTriangle);
    RenderEntity* QuadTextureData = RE_System.Get(RE_QuadTexture);
    RenderEntity* QuadUnicolorData = RE_System.Get(RE_QuadUnicolor);
    ASSERT(ColorTriangleData);
    ASSERT(QuadTextureData);
    ASSERT(QuadUnicolorData);
    float HalfWidth = GlobalState::Width * 0.5f;
    float HalfHeight = GlobalState::Height * 0.5f;
    m4f SpriteWorld = m4f::Scale(HalfWidth, HalfHeight, 1.0f) * m4f::Trans(0.0f, 0.0f, 0.0f);
    if (ColorTriangleData)
    {
        ColorTriangleData->bVisible = true;
        ColorTriangleData->World = SpriteWorld;
        ColorTriangleData->Type = DrawType::Color;
        ColorTriangleData->Mesh = &GfxImpl::MeshTriangle;
        ColorTriangleData->ColorState = {};
    }
    if (QuadTextureData)
    {
        QuadTextureData->bVisible = true;
        QuadTextureData->World = SpriteWorld;
        QuadTextureData->Type = DrawType::Texture;
        QuadTextureData->Mesh = &GfxImpl::MeshQuad;
        QuadTextureData->TextureState = { };
        QuadTextureData->TextureState.Texture = DefaultTexture;
        QuadTextureData->TextureState.TextureSRV = DefaultTextureSRV;
        QuadTextureData->TextureState.Sampler = DefaultSamplerState;
    }
    if (QuadUnicolorData)
    {
        QuadUnicolorData->bVisible = true;
        QuadUnicolorData->World = SpriteWorld;
        QuadUnicolorData->Type = DrawType::Unicolor;
        QuadUnicolorData->Mesh = &GfxImpl::MeshQuadMin;
        QuadUnicolorData->UnicolorState.Color = GetRandomColorDim();
    }
    // TODO: Check for correct init'd state here
    return true;
}

bool GfxImpl::Term()
{
    RE_System.Term();

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
    u32 SyncInterval = 0;
    u32 PresentFlags = 0;
    DXGI_PRESENT_PARAMETERS PresentParams = {};
    SwapChain->Present1(SyncInterval, PresentFlags, &PresentParams);
}

void UBG_Gfx_DX11::Draw()
{
    DrawBegin();
    GfxImpl::Draw(Context);
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

    bResult &= GfxImpl::Init(Device);

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

    u32 ShaderCompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
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
    size_t NumInputElements,
    u32 NumConstantBuffers,
    ID3D11Buffer** ConstantBuffers,
    u32 NumShaderRVs,
    ID3D11ShaderResourceView** ShaderRVs,
    u32 NumSamplers,
    ID3D11SamplerState** Samplers
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

    DrawStateT Result = {};

    if (VSBlob && PSBlob)
    {
        Device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &Result.VertexShader);
        Device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &Result.PixelShader);

        Device->CreateInputLayout(InputElements, (u32)NumInputElements, VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &Result.InputLayout);


    }
    SafeRelease(VSBlob);
    SafeRelease(PSBlob);

    ASSERT(!NumConstantBuffers || ConstantBuffers);
    ASSERT(!NumShaderRVs || ShaderRVs);
    ASSERT(!NumSamplers || Samplers);
    if (NumConstantBuffers && ConstantBuffers)
    {
        Result.NumConstantBuffers = NumConstantBuffers;
        for (u32 Idx = 0; Idx < NumConstantBuffers; Idx++)
        {
            Result.ConstantBuffers[Idx] = ConstantBuffers[Idx];
        }
    }
    if (NumShaderRVs && ShaderRVs)
    {
        Result.NumShaderRVs = NumShaderRVs;
        for (u32 Idx = 0; Idx < NumShaderRVs; Idx++)
        {
            Result.ShaderRVs[Idx] = ShaderRVs[Idx];
        }
    }
    if (NumSamplers && Samplers)
    {
        Result.NumSamplers = NumSamplers;
        for (u32 Idx = 0; Idx < NumSamplers; Idx++)
        {
            Result.Samplers[Idx] = Samplers[Idx];
        }
    }

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

