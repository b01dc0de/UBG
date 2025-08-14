#ifndef UBG_GFXDX11_H
#define UBG_GFXDX11_H

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

struct UBG_Gfx_DX11
{
    static ID3D11Device* Device;
    static ID3D11DeviceContext* Context;
    static D3D_FEATURE_LEVEL FeatureLevel;
    static IDXGISwapChain1* SwapChain;
    static ID3D11Texture2D* BackBuffer;
    static ID3D11RenderTargetView* RenderTargetView;
    static ID3D11RasterizerState* RasterState;
    static ID3D11Texture2D* DepthStencil;
    static ID3D11DepthStencilView* DepthStencilView;

    static void DrawBegin();
    static void DrawEnd();
    static void Draw();
    static bool Init();
    static bool Term();
};

using UBG_GfxT = UBG_Gfx_DX11;
using UBG_GfxContextT = ID3D11DeviceContext;

struct MeshStateT
{
    size_t VertexSize;
    size_t NumVerts;
    size_t NumInds;
    ID3D11Buffer* VxBuffer;
    ID3D11Buffer* IxBuffer;

    void Bind(UBG_GfxContextT* Context);
    void Draw(UBG_GfxContextT* Context);
    void SafeRelease();
};

enum struct DrawType
{
    Color,
    Texture,
    Unicolor
};

struct DrawColorState { }; // Nothing needed for now

struct DrawTextureState
{
    ID3D11Texture2D* Texture;
    ID3D11ShaderResourceView* TextureSRV;
    ID3D11SamplerState* Sampler;
};

struct DrawUnicolorState
{
    v4f Color;
};

using RenderEntityID = u32;

struct RenderEntity
{
    RenderEntityID ID;
    bool bVisible;
    m4f World;
    DrawType Type;
    MeshStateT* Mesh;
    union
    {
        DrawColorState ColorState;
        DrawTextureState TextureState;
        DrawUnicolorState UnicolorState;
    };

    void UpdateWorld(UBG_GfxContextT* Context);
    void Draw(UBG_GfxContextT* Context);
};

struct RenderEntitySystem
{
    static constexpr size_t MaxEntities = 1024;
    DArray<RenderEntity> Entities;
    RenderEntityID CounterID;

    void Init();
    void Term();
    RenderEntity* Get(RenderEntityID ID);
    RenderEntityID Create();
    void Destroy(RenderEntityID ID);
    void DrawAll(UBG_GfxContextT* Context);
};

#endif // UBG_GFXDX11_H

