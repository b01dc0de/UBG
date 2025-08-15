#include "UBG.h" // Includes UBGame.h

struct PlayerShip
{
    RenderEntityID idShip;
    v2f Pos;
    float Angle;

    void Init(GfxSystem* Gfx)
    {
        Pos = { 0.0f, 0.0f };
        Angle = 0.0f;

        ImageT PlayerShipTextureImage = {};
        LoadBMPFile("Assets/player_ship.bmp", PlayerShipTextureImage);
        ASSERT(PlayerShipTextureImage.PxBuffer);
        RenderEntity PlayerShipData = {};
        PlayerShipData.bVisible = true;
        PlayerShipData.World = m4f::Identity();
        PlayerShipData.Type = DrawType::Texture;
        PlayerShipData.idMesh = Gfx->idQuadTexture;
        PlayerShipData.TextureState.idTexture = Gfx->CreateTexture(&PlayerShipTextureImage);
        PlayerShipData.TextureState.idSampler = Gfx->idDefaultSampler;
        idShip = Gfx->CreateEntity(PlayerShipData);
        delete[] PlayerShipTextureImage.PxBuffer;

    }
    void Term(GfxSystem* Gfx)
    {
        Gfx->Entities.Destroy(idShip);
    }

    void Update(GfxSystem* Gfx)
    {
        bool bKeyW = KeyboardState::GetKey('W');
        bool bKeyA = KeyboardState::GetKey('A');
        bool bKeyS = KeyboardState::GetKey('S');
        bool bKeyD = KeyboardState::GetKey('D');
        constexpr float fSpeed = 00.1f;
        if (bKeyW != bKeyS)
        {
            Pos.Y += bKeyW ? fSpeed : -fSpeed;
        }
        if (bKeyA != bKeyD)
        {
            Pos.X += bKeyD ? fSpeed : -fSpeed;
        }

        if (!MouseState::bOffscreen)
        {
            int HalfWidth = GlobalEngine->Width / 2;
            int HalfHeight = GlobalEngine->Height / 2;
            v2f MousePos = { (float)(MouseState::MouseX - HalfWidth), (float)(MouseState::MouseY - HalfHeight) };
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
        pRent->World = m4f::Scale(100.0f, 100.0f, 1.0f) * m4f::RotZ(Angle) * m4f::Trans(Pos.X, Pos.Y, 0.0f);

        /*
        RenderEntity* pPlayerShip = Gfx->Entities.Get(idShip);
        ASSERT(pPlayerShip);
        float fTime = (float)ClockT::CurrTime;
        float HalfWidth = GlobalEngine->Width * 0.5f;
        float HalfHeight = GlobalEngine->Height * 0.5f;
        pPlayerShip->World = m4f::Scale(HalfWidth, HalfHeight, 1.0f) * m4f::Trans(sinf(fTime)*HalfWidth, cosf(fTime)*HalfHeight, 0.0f);
        */
    }
};

struct UBGameImpl
{
    GfxSystem Gfx;

    PlayerShip Player;
    RenderEntityID idBossShip;

    bool Init()
    {
        bool bResult = Gfx.Init((UBG_GfxT*)GlobalEngine->GfxState);

        /*
        float HalfWidth = GlobalEngine->Width * 0.5f;
        float HalfHeight = GlobalEngine->Height * 0.5f;
        m4f SpriteWorld = m4f::Scale(HalfWidth, HalfHeight, 1.0f) * m4f::Trans(0.0f, 0.0f, 0.0f);
        */

        Player.Init(&Gfx);

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
    void Draw(UBG_GfxContextT* Context)
    {
        Gfx.Entities.DrawAll(Context, &Gfx);
    }
    bool Term()
    {
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
    void Draw(UBG_GfxContextT* Context)
    {
        System.Entities.DrawAll(Context, &System);
    }
    bool Term()
    {
        bool bResult = System.Term();
        return bResult;
    }
};

#define RUN_DEMO() (0)
#if RUN_DEMO()
UBGameImplDemo* Impl = nullptr;
using UBGameImplT = UBGameImplDemo;
#else
UBGameImpl* Impl = nullptr;
using UBGameImplT = UBGameImpl;
#endif // RUN_DEMO()

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

void UBGame::Draw(UBG_GfxContextT* Context)
{
    Impl->Draw(Context);
}
