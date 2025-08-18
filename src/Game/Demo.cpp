#include "../UBG.h"
#include "Demo.h"

bool UBGameImplDemo::Init()
{
    bool bResult = System.Init((UBG_GfxT*)GlobalEngine->GfxState);

    // idMesh triangle:
    {
        VxColor TriangleVerts[] = {
            { { 0.0f, 0.5f, 0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { -0.5f, -0.5f, 0.5f, 1.0f}, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { +0.5f, -0.5f, 0.5f, 1.0f}, { 0.0f, 0.0f, 1.0f, 1.0f } },
        };

        unsigned int TriangleInds[] = { 0, 2, 1 };

        idTriangle = System.CreateMesh(
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

        idQuad = System.CreateMesh(
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

        idQuadMin = System.CreateMesh
        (
            sizeof(VxMin),
            ARRAY_SIZE(QuadMinVerts),
            QuadMinVerts,
            ARRAY_SIZE(QuadInds),
            QuadInds
        );
    }

    float HalfWidth = GlobalEngine->Width * 0.5f;
    float HalfHeight = GlobalEngine->Height * 0.5f;
    m4f SpriteWorld = m4f::Scale(HalfWidth, HalfHeight, 1.0f) * m4f::Trans(0.0f, 0.0f, 0.0f);

    RenderEntity ColorTriangleData = {};
    ColorTriangleData.bVisible = true;
    ColorTriangleData.World = SpriteWorld;
    ColorTriangleData.Type = DrawType::Color;
    ColorTriangleData.idMesh = idTriangle;
    ColorTriangleData.ColorState = {};
    idTriangleColor = System.CreateEntity(ColorTriangleData);

    RenderEntity QuadTextureData = {};
    QuadTextureData.bVisible = true;
    QuadTextureData.World = SpriteWorld;
    QuadTextureData.Type = DrawType::Texture;
    QuadTextureData.idMesh = idQuad;
    QuadTextureData.TextureState.idTexture = System.idFallbackTexture;
    QuadTextureData.TextureState.idSampler = System.idDefaultSampler;
    idQuadTexture = System.CreateEntity(QuadTextureData);

    RenderEntity QuadUnicolorData = {};
    QuadUnicolorData.bVisible = true;
    QuadUnicolorData.World = SpriteWorld;
    QuadUnicolorData.Type = DrawType::Unicolor;
    QuadUnicolorData.idMesh = idQuadMin;
    QuadUnicolorData.UnicolorState.Color = GetRandomColorDim();
    idQuadUnicolor = System.CreateEntity(QuadUnicolorData);

    return bResult;
}
void UBGameImplDemo::Update()
{
}
void UBGameImplDemo::Draw()
{
    System.Entities.DrawAll(&System);
}
bool UBGameImplDemo::Term()
{
    bool bResult = System.Term();
    return bResult;
}
