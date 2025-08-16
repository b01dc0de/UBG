#include "UBG.h" // Includes UBGame.h

struct PlayerShip
{
    TextureStateID idShipTexture;
    RenderEntityID idShip;
    v2f Pos;
    float Scale;
    float Angle;

    void Init(GfxSystem* Gfx)
    {
        Pos = { (float)GlobalEngine->Width*-0.25f, (float)GlobalEngine->Height*+0.25f };
        Scale = 100.0f;
        Angle = 0.0f;

        ImageT PlayerShipTextureImage = {};
        LoadBMPFile("Assets/player_ship.bmp", PlayerShipTextureImage);
        ASSERT(PlayerShipTextureImage.PxBuffer);
        RenderEntity PlayerShipData = {};
        PlayerShipData.bVisible = true;
        PlayerShipData.World = m4f::Identity();
        PlayerShipData.Type = DrawType::Texture;
        PlayerShipData.idMesh = Gfx->idQuadTexture;
        idShipTexture = Gfx->CreateTexture(&PlayerShipTextureImage);
        PlayerShipData.TextureState.idTexture = idShipTexture;
        PlayerShipData.TextureState.idSampler = Gfx->idDefaultSampler;
        idShip = Gfx->CreateEntity(PlayerShipData);
        delete[] PlayerShipTextureImage.PxBuffer;

    }
    void Term(GfxSystem* Gfx)
    {
        Gfx->Entities.Destroy(idShipTexture);
        Gfx->Entities.Destroy(idShip);
    }

    void Update(GfxSystem* Gfx)
    {
        bool bKeyW = GlobalEngine->Input->Keyboard.GetKey('W');
        bool bKeyA = GlobalEngine->Input->Keyboard.GetKey('A');
        bool bKeyS = GlobalEngine->Input->Keyboard.GetKey('S');
        bool bKeyD = GlobalEngine->Input->Keyboard.GetKey('D');
        constexpr f32 fSpeed = 100.1f;
        f32 AdjSpeed = fSpeed * (f32)GlobalEngine->Clock->LastFrameDuration;
        if (bKeyW != bKeyS)
        {
            Pos.Y += bKeyW ? AdjSpeed : -AdjSpeed;
        }
        if (bKeyA != bKeyD)
        {
            Pos.X += bKeyD ? AdjSpeed : -AdjSpeed;
        }
        float HalfScale = Scale * 0.5f;
        float HalfWidth = GlobalEngine->Width * 0.5f;
        float HalfHeight = GlobalEngine->Height * 0.5f;
        Pos.X = Clamp(0.0f + HalfScale - HalfWidth, GlobalEngine->Width - HalfScale - HalfWidth, Pos.X);
        Pos.Y = Clamp(0.0f + HalfScale - HalfHeight, GlobalEngine->Height - HalfScale - HalfHeight, Pos.Y);

        if (!GlobalEngine->Input->Mouse.bOffscreen)
        {
            v2f MousePos = { GlobalEngine->Input->Mouse.MouseX - HalfWidth, GlobalEngine->Input->Mouse.MouseY - HalfHeight };
            v2f Diff = { MousePos.X - Pos.X, MousePos.Y + Pos.Y };
            if (fIsZero(Length(Diff)))
            {
                Angle = 0.0f;
            }
            else
            {
                Angle = atan2f(Diff.Y, Diff.X);
            }
        }

        RenderEntity* pRent = Gfx->Entities.Get(idShip);
        ASSERT(pRent);
        pRent->World = m4f::Scale(Scale, Scale, 1.0f) * m4f::RotZ(Angle) * m4f::Trans(Pos.X, Pos.Y, 0.0f);
    }
};

struct BossShip
{
    RenderEntityID idShipMesh;
    RenderEntityID idShip;

    void Init(GfxSystem* Gfx)
    {
        constexpr u32 BossNumPoints = 32;
        constexpr size_t BossNumVerts = BossNumPoints + 1;
        constexpr size_t BossNumTris = BossNumPoints;
        constexpr size_t BossNumInds = BossNumTris * 3;
        constexpr float AverageRadius = 10.0f;
        constexpr float RadiusVariance = 5.0f;

        // Boss mesh:
        {
            VxMin BossVerts[BossNumVerts] = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            for (u32 Idx = 1; Idx < BossNumVerts; Idx++)
            {
                float Radius = AverageRadius + ((GetRandomFloatNorm() - 0.5f) * RadiusVariance);
                float Angle = (float)(Idx - 1) / (float)(BossNumVerts - 1) * fTAU;
                BossVerts[Idx] = { Radius * cosf(Angle), Radius * sinf(Angle), 0.0f, 1.0f };
            }

            u32 BossInds[BossNumTris * 3] = {};
            for (u32 TriIdx = 0; TriIdx < BossNumTris; TriIdx++)
            {
                size_t BaseIdx = TriIdx * 3;
                BossInds[BaseIdx + 0] = 0;
                BossInds[BaseIdx + 1] = TriIdx == BossNumTris - 1 ? 1 : TriIdx + 2;
                BossInds[BaseIdx + 2] = TriIdx + 1;
            }
            idShipMesh = Gfx->CreateMesh(sizeof(VxMin), BossNumVerts, BossVerts, BossNumInds, BossInds);
        }

        RenderEntity BossShipData = {};
        BossShipData.bVisible = true;
        BossShipData.World = m4f::Scale(10.0f, 10.0f, 1.0);
        BossShipData.Type = DrawType::Unicolor;
        BossShipData.idMesh = idShipMesh;
        BossShipData.UnicolorState.Color = v4f{ 1.0f, 0.0f, 0.0f, 1.0f };
        idShip = Gfx->CreateEntity(BossShipData);
    }
    void Term(GfxSystem* Gfx)
    {
        (void)Gfx;
    }
    void Update(GfxSystem* Gfx)
    {
        (void)Gfx;
    }
};

struct UBGameImpl
{
    GfxSystem Gfx;

    PlayerShip Player;
    BossShip Boss;

    bool Init()
    {
        bool bResult = Gfx.Init((UBG_GfxT*)GlobalEngine->GfxState);

        Player.Init(&Gfx);
        Boss.Init(&Gfx);

        /*
        RenderEntity BossShipData = {};
        BossShipData.bVisible = true;
        BossShipData.World = SpriteWorld;
        BossShipData.Type = DrawType::Unicolor;
        BossShipData.idMesh = idQuad;
        BossShipData.UnicolorState.Color = GetRandomColorDim();
        */
        return bResult;
    }
    void Update()
    {
        Player.Update(&Gfx);
    }
    void Draw()
    {
        Gfx.Entities.DrawAll(&Gfx);
    }
    bool Term()
    {
        Player.Term(&Gfx);
        Boss.Term(&Gfx);

        bool bResult = Gfx.Term();
        return bResult;
    }
};

struct UBGameImplDemo
{
    GfxSystem System;

    MeshStateID idTriangle;
    MeshStateID idQuad;
    MeshStateID idQuadMin;
    RenderEntityID idTriangleColor;
    RenderEntityID idQuadTexture;
    RenderEntityID idQuadUnicolor;

    bool Init()
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
    void Update()
    {
    }
    void Draw()
    {
        System.Entities.DrawAll(&System);
    }
    bool Term()
    {
        bool bResult = System.Term();
        return bResult;
    }
};

#define RUN_DEMO() (0)
#if RUN_DEMO()
using UBGameImplT = UBGameImplDemo;
#else
using UBGameImplT = UBGameImpl;
#endif // RUN_DEMO()
UBGameImplT* Impl = nullptr;

bool UBGame::Init()
{
    Impl = new UBGameImplT{};
    return Impl->Init();
}

bool UBGame::Term()
{
    bool bResult = Impl->Term();
    delete Impl;
    return bResult;
}

void UBGame::Update()
{
    Impl->Update();
}

void UBGame::Draw()
{
    Impl->Draw();
}
