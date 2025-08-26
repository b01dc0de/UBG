#ifndef UBG_GFXDX11_H
#define UBG_GFXDX11_H

static_assert(UBG_GFX_IMPL_DX11(), "UBG_GfxDX11.h: Header used without DX11 GfxBackend impl defined");

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
    ID3D11RasterizerState* DefaultRasterState;
    ID3D11RasterizerState* WireframeRasterState;
    ID3D11Texture2D* DepthStencil;
    ID3D11DepthStencilView* DepthStencilView;
    ID3D11BlendState* DefaultBlendState;

    void DrawBegin();
    void DrawEnd();
    void Draw();
    bool Init();
    bool Term();
};

using UBG_GfxT = UBG_Gfx_DX11;
using UBG_GfxContextT = ID3D11DeviceContext;

// Aliases for TypeID
// TODO: In the future we problably want to distinguish these
using MeshStateID = TypeID;
using MeshInstStateID = TypeID;
using RenderEntityID = TypeID;
using RenderInstEntityID = TypeID;
using ShaderBufferID = TypeID;
using TextureStateID = TypeID;
using SamplerStateID = TypeID;
using DrawStateID = TypeID;

// Forward declare GfxSystem:
struct GfxSystem;

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

    void Bind(GfxSystem* System);
    void Bind(GfxSystem* System, u32 InNumShaderRVs, ID3D11ShaderResourceView** ShaderRVs, u32 InNumSamplers, ID3D11SamplerState** Samplers);
    void SafeRelease();
};

struct MeshStateT
{
    size_t VertexSize;
    size_t NumVerts;
    size_t NumInds;
    D3D_PRIMITIVE_TOPOLOGY TopType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    ID3D11Buffer* VxBuffer;
    ID3D11Buffer* IxBuffer;

    void Bind(UBG_GfxContextT* Context);
    void Draw(UBG_GfxContextT* Context);
    void SafeRelease();
};

struct MeshInstStateT
{
    static constexpr size_t DefaultMaxInstCount = 1024;

    size_t VertexSize = 0;
    size_t NumVerts = 0;
    size_t NumInds = 0;
    size_t PerInstSize = 0;
    size_t MaxInstCount = 0;
    size_t InstBufferSize = 0;
    D3D_PRIMITIVE_TOPOLOGY TopType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    ID3D11Buffer* VxBuffer = nullptr;
    ID3D11Buffer* InstBuffer = nullptr;
    ID3D11Buffer* IxBuffer = nullptr;

    void Bind(UBG_GfxContextT* Context);
    void Draw(UBG_GfxContextT* Context, size_t NumInst, void* pInstData);
    void SafeRelease();
};

struct TextureStateT
{
    ID3D11Texture2D* Texture;
    ID3D11ShaderResourceView* SRV;

    void SafeRelease();
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

enum struct DrawInstType
{
    Color,
    Count
};

struct DrawColorState { }; // Nothing needed for now

struct DrawTextureState
{
    TextureStateID idTexture;
    SamplerStateID idSampler;
};

struct DrawUnicolorState
{
    v4f Color;
};

struct RenderEntity
{
    bool bVisible = true;
    bool bWireframe = false;
    u8 StageIndex = 0;
    m4f World = m4f::Identity();
    DrawType Type = DrawType::Count;
    MeshStateID idMesh = 0;
    union
    {
        DrawColorState ColorState;
        DrawTextureState TextureState;
        DrawUnicolorState UnicolorState;
    };

    static RenderEntity Default(m4f World, DrawType Type, MeshStateID idMesh);

    void UpdateWorld(GfxSystem* System);
    void Draw(GfxSystem* System);
};

struct InstRectColorData
{
    RectF Rect;
    v4f Color;
};

struct RenderInstEntity
{
    bool bVisible = true;
    bool bWireframe = false;
    u8 StageIndex = 0;
    m4f World = m4f::Identity();
    DrawInstType Type = DrawInstType::Count;
    MeshInstStateID idMesh = 0;
    size_t NumInst = 0;
    void* pInstData = nullptr;

    void UpdateWorld(GfxSystem* System);
    void Draw(GfxSystem* System);
};

// TODO: Implement _some_ way of enforcing a draw order
struct RenderEntitySystem
{
    ListID<RenderEntity> Entities;
    ListID<RenderInstEntity> EntitiesInst;

    void Init();
    void Term();
    RenderEntity* Get(RenderEntityID ID);
    RenderInstEntity* GetInst(RenderInstEntityID ID);
    RenderEntityID Create();
    RenderEntityID Create(RenderEntity InitData);
    RenderInstEntityID CreateInst();
    RenderInstEntityID CreateInst(RenderInstEntity InitData);
    void Destroy(RenderEntityID ID);
    void DestroyInst(RenderInstEntityID ID);
    void DrawAll(GfxSystem* System);
};

struct GfxSystem
{
    UBG_GfxT* Backend;

    RenderEntitySystem Entities;

    DrawStateID idsDrawState[(size_t)DrawType::Count];
    ListID<DrawStateT> DrawStates;
    DrawStateID idsDrawInstState[(size_t)DrawInstType::Count];
    ListID<DrawStateT> DrawInstStates;

    ShaderBufferID idWorldBuffer;
    ShaderBufferID idViewProjBuffer;
    ShaderBufferID idUnicolorBuffer;
    ListID<ShaderBufferT> ShaderBuffers;

    TextureStateID idFallbackTexture;
    ListID<TextureStateT> Textures;
    SamplerStateID idDefaultSampler;
    ListID<SamplerStateT> Samplers;

    MeshStateID idQuadTexture;
    MeshStateID idQuadUnicolor;
    ListID<MeshStateT> Meshes;
    MeshInstStateID idInstRectColor;
    MeshInstStateID idInstRectColorLines;
    ListID<MeshInstStateT> MeshesInst;

    Camera MainCameraO;

    void Draw();
    bool Init(UBG_GfxT* _GfxBackend);
    bool Term();

    RenderEntity* GetEntity(RenderEntityID ID);
    RenderEntityID CreateEntity(RenderEntity EntityState);
    RenderInstEntity* GetEntityInst(RenderInstEntityID ID);
    RenderInstEntityID CreateEntityInst(RenderInstEntity EntityState);
    void DestroyEntity(RenderEntityID ID);
    void DestroyEntityInst(RenderInstEntityID ID);
    MeshStateT* GetMesh(MeshStateID ID);
    MeshInstStateT* GetMeshInst(MeshInstStateID ID);
    MeshStateID CreateMesh(size_t VertexSize, size_t NumVertices, void* VertexData, size_t NumIndices, u32* IndexData, D3D11_PRIMITIVE_TOPOLOGY TopType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    MeshInstStateID CreateMeshInst(size_t VertexSize, size_t PerInstSize, size_t MaxInstCount, size_t NumVertices, void* VertexData, size_t NumIndices, u32* IndexData, D3D11_PRIMITIVE_TOPOLOGY TopType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    void DestroyMesh(MeshStateID ID);
    void DestroyMeshInst(MeshInstStateID ID);
    TextureStateT* GetTexture(TextureStateID ID);
    TextureStateID CreateTexture(ImageT* Image);
    void DestroyTexture(TextureStateID ID);
};

#endif // UBG_GFXDX11_H

