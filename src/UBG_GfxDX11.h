#ifndef UBG_GFXDX11_H
#define UBG_GFXDX11_H

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

struct UBG_Gfx_DX11
{
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    D3D_FEATURE_LEVEL FeatureLevel;
    IDXGISwapChain1* SwapChain;
    ID3D11Texture2D* BackBuffer;
    ID3D11RenderTargetView* RenderTargetView;
    ID3D11RasterizerState* RasterState;
    ID3D11Texture2D* DepthStencil;
    ID3D11DepthStencilView* DepthStencilView;

    void DrawBegin();
    void DrawEnd();
    void Draw();
    bool Init();
    bool Term();
};

using UBG_GfxT = UBG_Gfx_DX11;
using UBG_GfxContextT = ID3D11DeviceContext;

// Aliases for TypeID, TODO: In the future we problably want to distinguish these
using MeshStateID = TypeID;
using RenderEntityID = TypeID;
using ShaderBufferID = TypeID;
using TextureStateID = TypeID;
using SamplerStateID = TypeID;
using DrawStateID = TypeID;

// Forward declare GfxSystem:
struct GfxSystem;
// TODO(Chris): We probably just want to pass GfxSystem in rather than UBG_GfxContextT for the ensuing function signatures...

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
    ShaderBufferID idsConstantBuffers[MaxConstantBuffers];

    void Bind(UBG_GfxContextT* Context, GfxSystem* System);
    void Bind(UBG_GfxContextT* Context, GfxSystem* System, u32 InNumShaderRVs, ID3D11ShaderResourceView** ShaderRVs, u32 InNumSamplers, ID3D11SamplerState** Samplers);
    void SafeRelease();
};

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

struct TextureStateT
{
    ID3D11Texture2D* Texture;
    ID3D11ShaderResourceView* SRV;
};

using SamplerStateT = ID3D11SamplerState*;

using ShaderBufferT = ID3D11Buffer*;

enum struct DrawType
{
    Color,
    Texture,
    Unicolor,
    Count
};

struct DrawColorState { }; // Nothing needed for now

struct DrawTextureState
{
    /*
    TextureStateT Texture;
    SamplerStateT Sampler;
    */
    TextureStateID idTexture;
    SamplerStateID idSampler;
};

struct DrawUnicolorState
{
    v4f Color;
};

struct RenderEntity
{
    bool bVisible;
    m4f World;
    DrawType Type;
    MeshStateID idMesh;
    union
    {
        DrawColorState ColorState;
        DrawTextureState TextureState;
        DrawUnicolorState UnicolorState;
    };

    void UpdateWorld(UBG_GfxContextT* Context, GfxSystem* System);
    void Draw(UBG_GfxContextT* Context, GfxSystem* System);
};

struct RenderEntitySystem
{
    ListID<RenderEntity> Entities;

    void Init();
    void Term();
    RenderEntity* Get(RenderEntityID ID);
    RenderEntityID Create();
    RenderEntityID Create(RenderEntity InitData);
    void Destroy(RenderEntityID ID);
    void DrawAll(UBG_GfxContextT* Context, GfxSystem* System);
};

struct GfxSystem
{
    UBG_GfxT* GfxBackend;

    RenderEntitySystem Entities;

    DrawStateID idsDrawState[(size_t)DrawType::Count];
    ListID<DrawStateT> DrawStates;

    ShaderBufferID idWorldBuffer;
    ShaderBufferID idViewProjBuffer;
    ShaderBufferID idUnicolorBuffer;
    ListID<ShaderBufferT> ShaderBuffers;

    TextureStateID idFallbackTexture;
    ListID<TextureStateT> Textures;
    SamplerStateID idDefaultSampler;
    ListID<SamplerStateT> Samplers;

    ListID<MeshStateT> Meshes;

    Camera MainCameraO;

    // TODO: Move these into UBGame
    MeshStateID idTriangle;
    MeshStateID idQuad;
    MeshStateID idQuadMin;
    RenderEntityID idTriangleColor;
    RenderEntityID idQuadTexture;
    RenderEntityID idQuadUnicolor;

    bool Init(UBG_GfxT* _GfxBackend);
    bool Term();

    TextureStateID CreateTexture(ImageT* Image);
    void DestroyTexture(TextureStateID ID);
};

#endif // UBG_GFXDX11_H

