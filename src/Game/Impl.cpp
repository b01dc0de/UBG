#include "../UBG.h"
#include "Impl.h"

void PlayerShip::Hit(UBGameImpl* Game)
{
    UNUSED_VAR(Game);
    Health -= 25.0f;
    if (Health <= 0.0f)
    {
        Term(Game);
        Init(Game);
    }
}

void PlayerShip::Init(UBGameImpl* Game)
{
    Pos = { (float)GlobalEngine->Width * -0.25f, (float)GlobalEngine->Height * +0.25f };
    Momentum = 0.0f;
    Scale = 25.0f;
    Angle = 0.0f;
    Health = 100.0f;
    float HalfScale = Scale * 0.5f;
    BoundingBox =
    {
        { Pos.X - HalfScale, Pos.Y - HalfScale }, // Min
        { Pos.X + HalfScale, Pos.Y + HalfScale } // Max
    };

    RenderEntity PlayerShipData = {};
    PlayerShipData.bVisible = true;
    PlayerShipData.World = m4f::Identity();

    {
        RenderEntity HealthbarData = {};
        HealthbarData.bVisible = true;
        HealthbarData.World = m4f::Identity();
        HealthbarData.Type = DrawType::Unicolor;
        HealthbarData.idMesh = Game->Gfx.idQuadUnicolor;
        HealthbarData.UnicolorState = { { 1.0f, 1.0f, 1.0f, 1.0f } };

        idShipHealthbarBG = Game->Gfx.CreateEntity(HealthbarData);
        ASSERT(idShipHealthbarBG);

        HealthbarData.UnicolorState = { { 1.0f, 0.0f, 0.0f, 1.0f } };
        idShipHealthbar = Game->Gfx.CreateEntity(HealthbarData);
        ASSERT(idShipHealthbar);
    }

    if (bUseShipMesh)
    {
        constexpr size_t NumVerts = 15;
        constexpr size_t NumTris = 7;
        constexpr size_t NumInds = NumTris * 3;
        static constexpr bool bCloseBox = true;

        float AngleStride = fPI * 2.0f / 3.0f;
        float AngleA = 0.0f;
        float AngleB = AngleA + AngleStride;
        float AngleC = AngleB + AngleStride;
        v2f PosA = { cosf(AngleA), sinf(AngleA) };
        v2f PosB = { cosf(AngleB), sinf(AngleB) };
        v2f PosC = { cosf(AngleC), sinf(AngleC) };
        float SideWidth = 1.0f - PosB.Y;
        float BoxTopY = bCloseBox ? PosB.Y : PosB.Y + SideWidth;
        float BoxLeftX = bCloseBox ? PosB.X : PosA.X - 2.0f;
        float BoxBotY = bCloseBox ? PosC.Y : PosC.Y - SideWidth;
        VxMin Verts[NumVerts] = { };
        Verts[0] = { PosA.X, PosA.Y, 0.0f, 1.0f };
        Verts[1] = { PosB.X, PosB.Y, 0.0f, 1.0f };
        Verts[2] = { PosC.X, PosC.Y, 0.0f, 1.0f };

        Verts[3] = { BoxLeftX, BoxTopY + SideWidth, 0.0f, 1.0f };
        Verts[4] = { PosA.X, BoxTopY + SideWidth, 0.0f, 1.0f };
        Verts[5] = { BoxLeftX, BoxTopY, 0.0f, 1.0f };
        Verts[6] = { PosA.X, BoxTopY, 0.0f, 1.0f };

        Verts[7] = { BoxLeftX, BoxBotY, 0.0f, 1.0f };
        Verts[8] = { PosA.X, BoxBotY, 0.0f, 1.0f };
        Verts[9] = { BoxLeftX, BoxBotY - SideWidth, 0.0f, 1.0f };
        Verts[10] = { PosA.X, BoxBotY - SideWidth, 0.0f, 1.0f };

        Verts[11] = { BoxLeftX - SideWidth, PosB.Y + SideWidth, 0.0f, 1.0f };
        Verts[12] = { BoxLeftX, PosB.Y + SideWidth, 0.0f, 1.0f };
        Verts[13] = { BoxLeftX - SideWidth, PosC.Y - SideWidth, 0.0f, 1.0f };
        Verts[14] = { BoxLeftX, PosC.Y - SideWidth, 0.0f, 1.0f };

        u32 Inds[NumInds] = {
            0, 2, 1,

            3, 4, 5,
            4, 6, 5,

            7, 8, 9,
            8, 10, 9,

            11, 12, 13,
            12, 14, 13
        };
        idShipMesh = Game->Gfx.CreateMesh(sizeof(VxMin), NumVerts, Verts, NumInds, Inds);
        ASSERT(idShipMesh);
        PlayerShipData.Type = DrawType::Unicolor;
        PlayerShipData.idMesh = idShipMesh;
        PlayerShipData.UnicolorState = { v4f{ 1.0f, 1.0f, 1.0f, 1.0f } };
    }
    else
    {
        ImageT PlayerShipTextureImage = {};
        LoadBMPFile("Assets/player_ship.bmp", PlayerShipTextureImage);
        ASSERT(PlayerShipTextureImage.PxBuffer);
        PlayerShipData.Type = DrawType::Texture;
        PlayerShipData.idMesh = Game->Gfx.idQuadTexture;
        idShipTexture = Game->Gfx.CreateTexture(&PlayerShipTextureImage);
        ASSERT(idShipTexture);
        PlayerShipData.TextureState.idTexture = idShipTexture;
        PlayerShipData.TextureState.idSampler = Game->Gfx.idDefaultSampler;
        delete[] PlayerShipTextureImage.PxBuffer;
    }
    idShip = Game->Gfx.CreateEntity(PlayerShipData);
    ASSERT(idShip);
}

void PlayerShip::Term(UBGameImpl* Game)
{
    if (bUseShipMesh)
    {
        Game->Gfx.DestroyMesh(idShipMesh);
    }
    else
    {
        Game->Gfx.DestroyTexture(idShipTexture);
    }
    Game->Gfx.DestroyEntity(idShip);
    Game->Gfx.DestroyEntity(idShipHealthbar);
    Game->Gfx.DestroyEntity(idShipHealthbarBG);
}

void PlayerShip::Update(UBGameImpl* Game)
{
    bool bKeyW = GlobalEngine->Input->Keyboard.GetKey('W');
    bool bKeyA = GlobalEngine->Input->Keyboard.GetKey('A');
    bool bKeyS = GlobalEngine->Input->Keyboard.GetKey('S');
    bool bKeyD = GlobalEngine->Input->Keyboard.GetKey('D');
    f32 DeltaTime = (f32)GlobalEngine->Clock->LastFrameDuration;
    bool bMoving = false;
    v2f Vel = { 0.0f, 0.0f };
    if (bKeyW != bKeyS && bKeyA != bKeyD)
    {
        bMoving = true;
        f32 AdjMom = Momentum / sqrtf(2.0f);
        Vel.Y = (bKeyS ? -AdjMom: +AdjMom) * fMaxSpeed;
        Vel.X = (bKeyA ? -AdjMom: +AdjMom) * fMaxSpeed;
    }
    else if (bKeyW != bKeyS)
    {
        bMoving = true;
        Vel.Y = (bKeyS ? -Momentum : +Momentum) * fMaxSpeed;
    }
    else if (bKeyA != bKeyD)
    {
        bMoving = true;
        Vel.X = (bKeyA ? -Momentum : +Momentum) * fMaxSpeed;
    }

    if (bMoving)
    {
        Momentum = Clamp(0.0f, 1.0f, Momentum + DeltaTime * 2.0f);
    }
    else
    {
        Momentum = Clamp(0.0f, 1.0f, Momentum - DeltaTime * 0.1f);
    }
    Pos.X += Vel.X * DeltaTime;
    Pos.Y += Vel.Y * DeltaTime;
    float HalfScale = Scale * 0.5f;
    float HalfWidth = GlobalEngine->Width * 0.5f;
    float HalfHeight = GlobalEngine->Height * 0.5f;
    Pos.X = Clamp(0.0f + HalfScale - HalfWidth, GlobalEngine->Width - HalfScale - HalfWidth, Pos.X);
    Pos.Y = Clamp(0.0f + HalfScale - HalfHeight, GlobalEngine->Height - HalfScale - HalfHeight, Pos.Y);
    BoundingBox =
    {
        { Pos.X - HalfScale, Pos.Y - HalfScale }, // Min
        { Pos.X + HalfScale, Pos.Y + HalfScale } // Max
    };

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

    RenderEntity* pRent = Game->Gfx.GetEntity(idShip);
    ASSERT(pRent);
    pRent->World = m4f::Scale(Scale, Scale, 1.0f) * m4f::RotZ(Angle) * m4f::Trans(Pos.X, Pos.Y, 0.0f);

    // Health bar:
    {
        v2f HealthbarPos = { 256.0f, 256.0f };
        v2f HealthbarSize = { 100.0f, 30.0f };

        RenderEntity* pREHealthbarBG = Game->Gfx.GetEntity(idShipHealthbarBG);
        ASSERT(pREHealthbarBG);
        pREHealthbarBG->World = m4f::Scale(HealthbarSize.X, HealthbarSize.Y, 1.0f) * m4f::Trans(HealthbarPos.X, HealthbarPos.Y, 0.0f);

        RenderEntity* pREHealthbar = Game->Gfx.GetEntity(idShipHealthbar);
        ASSERT(pREHealthbar);
        pREHealthbar->World = m4f::Scale(HealthbarSize.X*Health/100.0f*0.90f, HealthbarSize.Y*0.90f, 1.0f) * m4f::Trans(HealthbarPos.X, HealthbarPos.Y, -0.1f);
    }

    static f32 LastBulletSpawn = 0.0f;
    static constexpr f32 SecondsPerBullet = 0.25f;
    static constexpr f32 Speed = 100.0f;
    f32 CurrTime = (f32)GlobalEngine->Clock->CurrTime;
    if (GlobalEngine->Input->Mouse.LeftButton)
    {
        if ((CurrTime - LastBulletSpawn) > SecondsPerBullet)
        {
            v2f Dir = { cosf(Angle) * Speed, -sinf(Angle) * Speed };
            Game->BulletMgr.NewBullet(Game, BulletType::Player, Pos, Dir);
            LastBulletSpawn = CurrTime;
        }
    }
}

void BossShip::Hit(UBGameImpl* Game)
{
    Health -= 1.0f;
    if (Health < 0.0f)
    {
        Term(Game);
        Init(Game);
    }
}

void BossShip::Init(UBGameImpl* Game)
{
    constexpr u32 BossNumPoints = 32;
    constexpr size_t BossNumVerts = BossNumPoints + 1;
    constexpr size_t BossNumTris = BossNumPoints;
    constexpr size_t BossNumInds = BossNumTris * 3;
    constexpr float AverageRadius = 10.0f;
    constexpr float RadiusVariance = 5.0f;

    BoundingBox = { {}, {} };
    Scale = 10.0f;
    Health = 100.0f;
    // Boss mesh:
    {
        VxMin BossVerts[BossNumVerts] = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        for (u32 Idx = 1; Idx < BossNumVerts; Idx++)
        {
            float Radius = AverageRadius + ((GetRandomFloatNorm() - 0.5f) * RadiusVariance);
            float Angle = (float)(Idx - 1) / (float)(BossNumVerts - 1) * fTAU;
            float X = Radius * cosf(Angle);
            float Y = Radius * sinf(Angle);
            BossVerts[Idx] = { X, Y, 0.0f, 1.0f };

            if (X < BoundingBox.Min.X) { BoundingBox.Min.X = X; }
            else if (X > BoundingBox.Max.X) { BoundingBox.Max.X = X; }
            if (Y < BoundingBox.Min.Y) { BoundingBox.Min.Y = Y; }
            else if (Y > BoundingBox.Max.Y) { BoundingBox.Max.Y = Y; }
        }

        u32 BossInds[BossNumTris * 3] = {};
        for (u32 TriIdx = 0; TriIdx < BossNumTris; TriIdx++)
        {
            size_t BaseIdx = TriIdx * 3;
            BossInds[BaseIdx + 0] = 0;
            BossInds[BaseIdx + 1] = TriIdx == BossNumTris - 1 ? 1 : TriIdx + 2;
            BossInds[BaseIdx + 2] = TriIdx + 1;
        }
        idShipMesh = Game->Gfx.CreateMesh(sizeof(VxMin), BossNumVerts, BossVerts, BossNumInds, BossInds);
        ASSERT(idShipMesh);
    }
    // Boss BoundingBox mesh:
    {
        constexpr int NumVerts = 16;
        constexpr int NumTris = 8;
        constexpr int NumInds = NumTris * 3;
        VxMin BossVertsBB[NumVerts] = {};
        constexpr f32 WidthBB = 0.25f;
        BossVertsBB[0] = { BoundingBox.Min.X - WidthBB, BoundingBox.Max.Y, 0.5f, 1.0f };
        BossVertsBB[1] = { BoundingBox.Min.X, BoundingBox.Max.Y, 0.5f, 1.0f };
        BossVertsBB[2] = { BoundingBox.Min.X - WidthBB, BoundingBox.Min.Y, 0.5f, 1.0f };
        BossVertsBB[3] = { BoundingBox.Min.X, BoundingBox.Min.Y, 0.5f, 1.0f };

        BossVertsBB[4] = { BoundingBox.Max.X, BoundingBox.Max.Y, 0.5f, 1.0f };
        BossVertsBB[5] = { BoundingBox.Max.X + WidthBB, BoundingBox.Max.Y, 0.5f, 1.0f };
        BossVertsBB[6] = { BoundingBox.Max.X, BoundingBox.Min.Y, 0.5f, 1.0f };
        BossVertsBB[7] = { BoundingBox.Max.X + WidthBB, BoundingBox.Min.Y, 0.5f, 1.0f };

        BossVertsBB[8] = { BoundingBox.Min.X, BoundingBox.Max.Y + WidthBB, 0.5f, 1.0f };
        BossVertsBB[9] = { BoundingBox.Max.X, BoundingBox.Max.Y + WidthBB, 0.5f, 1.0f };
        BossVertsBB[10] = { BoundingBox.Min.X, BoundingBox.Max.Y, 0.5f, 1.0f };
        BossVertsBB[11] = { BoundingBox.Max.X, BoundingBox.Max.Y, 0.5f, 1.0f };

        BossVertsBB[12] = { BoundingBox.Min.X, BoundingBox.Min.Y, 0.5f, 1.0f };
        BossVertsBB[13] = { BoundingBox.Max.X, BoundingBox.Min.Y, 0.5f, 1.0f };
        BossVertsBB[14] = { BoundingBox.Min.X, BoundingBox.Min.Y - WidthBB, 0.5f, 1.0f };
        BossVertsBB[15] = { BoundingBox.Max.X, BoundingBox.Min.Y - WidthBB, 0.5f, 1.0f };

        u32 BossIndsBB[NumInds] = {
            0, 1, 2,
            1, 3, 2,

            4, 5, 6,
            5, 7, 6,

            8, 9, 10,
            9, 11, 10,

            12, 13, 14,
            13, 15, 14
        };

        idBoundingBoxMesh = Game->Gfx.CreateMesh
        (
            sizeof(VxMin),
            ARRAY_SIZE(BossVertsBB), BossVertsBB,
            ARRAY_SIZE(BossIndsBB), BossIndsBB
        );
        ASSERT(idBoundingBoxMesh);

        RenderEntity BoundingBoxRenderData = {};
        BoundingBoxRenderData.bVisible = true;
        BoundingBoxRenderData.World = m4f::Scale(Scale, Scale, 1.0f); // m4f::Scale(50.0f, 50.0f, 1.0f);
        BoundingBoxRenderData.Type = DrawType::Unicolor;
        BoundingBoxRenderData.idMesh = idBoundingBoxMesh;
        BoundingBoxRenderData.UnicolorState = { { 1.0f, 1.0f, 0.0f, 1.0f } };
        idBoundingBox = Game->Gfx.CreateEntity(BoundingBoxRenderData);
    }

    RenderEntity BossShipData = {};
    BossShipData.bVisible = true;
    BossShipData.World = m4f::Scale(Scale, Scale, 1.0);
    BossShipData.Type = DrawType::Unicolor;
    BossShipData.idMesh = idShipMesh;
    BossShipData.UnicolorState.Color = v4f{ 1.0f, 0.0f, 0.0f, 1.0f };
    idShip = Game->Gfx.CreateEntity(BossShipData);
    ASSERT(idShip);
}

void BossShip::Term(UBGameImpl* Game)
{
    Game->Gfx.DestroyEntity(idBoundingBox);
    Game->Gfx.DestroyMesh(idBoundingBoxMesh);
    Game->Gfx.DestroyEntity(idShip);
    Game->Gfx.DestroyMesh(idShipMesh);
}

void BossShip::Update(UBGameImpl* Game)
{
    static f32 LastBulletSpawn = 0.0f;
    static constexpr f32 SecondsPerBullet = 0.25f;
    static f32 SpawnDir = 0.0f;
    static f32 TurnSpeed = SecondsPerBullet * fPI;
    static constexpr f32 Speed = 100.0f;
    f32 CurrTime = (f32)GlobalEngine->Clock->CurrTime;
    if ((CurrTime - LastBulletSpawn) > SecondsPerBullet)
    {
        v2f Dir = { cosf(SpawnDir) * Speed, sinf(SpawnDir) * Speed };
        SpawnDir -= TurnSpeed;
        Game->BulletMgr.NewBullet(Game, BulletType::Boss, v2f{ 0.0f, 0.0f }, Dir);
        LastBulletSpawn = CurrTime;
    }
}

bool BulletManager::DoesCollide(PerBulletData& Bullet, AABB* BoundingBox)
{
    SphereBB BulletBB = { Bullet.Pos, Bullet.Type == BulletType::Player ? PlayerBulletSize : BossBulletSize };
    return Collision::Check(BoundingBox, &BulletBB);
}

bool BulletManager::IsOffscreen(PerBulletData& Bullet)
{
    float HalfWidth = GlobalEngine->Width * 0.5f;
    float HalfHeight = GlobalEngine->Height * 0.5f;
    return Bullet.Pos.X < -HalfWidth || Bullet.Pos.X > +HalfWidth ||
        Bullet.Pos.Y < -HalfHeight || Bullet.Pos.Y > +HalfHeight;
}

void BulletManager::NewBullet(UBGameImpl* Game, BulletType Type, v2f Pos, v2f Vel)
{
    if ((Type == BulletType::Player && NumBulletsPlayer < MaxBulletsPlayer) ||
        (Type == BulletType::Boss && NumBulletsBoss < MaxBulletsBoss))
    {
        if (InactiveBullets.Num)
        {
            RenderEntityID NewID = InactiveBullets[0];
            if (NewID == 0)
            {
                DebugBreak();
            }
            InactiveBullets.Remove(0);
            PerBulletData NewBullet =
            {
                NewID,
                Type,
                Pos,
                Vel
            };
            ActiveBullets.Add(NewBullet);
            RenderEntity* BulletRE = Game->Gfx.GetEntity(NewID);
            ASSERT(BulletRE);
            BulletRE->bVisible = true;
            BulletRE->World = m4f::Identity();
            BulletRE->Type = DrawType::Unicolor;
            BulletRE->idMesh = idBulletMesh;
            BulletRE->UnicolorState = { Type == BulletType::Player ? PlayerBulletColor : BossBulletColor };

            if (bDebugPrint)
            {
                Outf("[debug][BulletManager]: Recycled NewBullet\n");
                Outf("\tRenderEntityID: %d\n", NewID);
                Outf("\tType: %s\n", Type == BulletType::Player ? "Player" : "Boss");
                Outf("\tPos: <%0.2f, %0.2f>\n", Pos.X, Pos.Y);
                Outf("\tDir: <%0.2f, %0.2f>\n", Vel.X, Vel.Y);
            }
        }
        else
        {
            RenderEntity REBulletData = { };
            REBulletData.bVisible = true;
            REBulletData.World = m4f::Identity();
            REBulletData.Type = DrawType::Unicolor;
            REBulletData.idMesh = idBulletMesh;
            REBulletData.UnicolorState = { Type == BulletType::Player ? PlayerBulletColor : BossBulletColor };
            RenderEntityID NewID = Game->Gfx.CreateEntity(REBulletData);
            if (NewID == 0)
            {
                DebugBreak();
            }
            PerBulletData NewBullet = { NewID, Type, Pos, Vel };
            ActiveBullets.Add(NewBullet);
            if (bDebugPrint)
            {
                Outf("[debug][BulletManager]: Created NewBullet\n");
                Outf("\tRenderEntityID: %d\n", NewID);
                Outf("\tType: %s\n", Type == BulletType::Player ? "Player" : "Boss");
                Outf("\tPos: <%0.2f, %0.2f>\n", Pos.X, Pos.Y);
                Outf("\tDir: <%0.2f, %0.2f>\n", Vel.X, Vel.Y);
            }
        }

        if (Type == BulletType::Player)
        {
            NumBulletsPlayer++;
        }
        else
        {
            NumBulletsBoss++;
        }
    }
    else
    {
        if (bDebugPrint)
        {
            Outf("[debug][BulletManager]: DIDN'T create NewBullet: MaxBullets reached\n");
            Outf("\tWould be stats:\n");
            Outf("\tType: %s\n", Type == BulletType::Player ? "Player" : "Boss");
            Outf("\tPos: <%0.2f, %0.2f>\n", Pos.X, Pos.Y);
            Outf("\tDir: <%0.2f, %0.2f>\n", Vel.X, Vel.Y);
        }
    }

}

void BulletManager::Init(UBGameImpl* Game)
{
    NumBulletsPlayer = 0;
    NumBulletsBoss = 0;

    // Bullet mesh:
    {
        constexpr u32 NumPoints = 8;
        constexpr size_t NumVerts = NumPoints + 1;
        constexpr size_t NumTris = NumPoints;
        constexpr size_t NumInds = NumTris * 3;

        VxMin Verts[NumVerts] = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        for (u32 Idx = 1; Idx < NumVerts; Idx++)
        {
            float Angle = (float)(Idx - 1) / (float)(NumVerts - 1) * fTAU;
            Verts[Idx] = { cosf(Angle), sinf(Angle), 0.0f, 1.0f };
        }

        u32 Inds[NumTris * 3] = {};
        for (u32 TriIdx = 0; TriIdx < NumTris; TriIdx++)
        {
            size_t BaseIdx = TriIdx * 3;
            Inds[BaseIdx + 0] = 0;
            Inds[BaseIdx + 1] = TriIdx == NumTris - 1 ? 1 : TriIdx + 2;
            Inds[BaseIdx + 2] = TriIdx + 1;
        }

        idBulletMesh = Game->Gfx.CreateMesh(sizeof(VxMin), NumVerts, Verts, NumInds, Inds);
    }
}

void BulletManager::Term(UBGameImpl* Game)
{
    Game->Gfx.DestroyMesh(idBulletMesh);
    for (size_t Idx = 0; Idx < ActiveBullets.Num; Idx++)
    {
        Game->Gfx.DestroyEntity(ActiveBullets[Idx].ID);
    }
    for (size_t Idx = 0; Idx < InactiveBullets.Num; Idx++)
    {
        Game->Gfx.DestroyEntity(InactiveBullets[Idx]);
    }
}

void BulletManager::Update(UBGameImpl* Game)
{
    for (size_t Idx = 0; Idx < ActiveBullets.Num; Idx++)
    {
        PerBulletData& Bullet = ActiveBullets[Idx];
        f32 dt = (f32)GlobalEngine->Clock->LastFrameDuration;
        v2f AdjVel = { Bullet.Vel.X * dt, Bullet.Vel.Y * dt };
        Bullet.Pos = Bullet.Pos + AdjVel;

        RenderEntity* BulletRE = Game->Gfx.GetEntity(Bullet.ID);
        ASSERT(BulletRE);

        bool bHitsPlayer = Bullet.Type == BulletType::Player ? false : DoesCollide(Bullet, &Game->Player.BoundingBox);
        bool bHitsBoss = Bullet.Type == BulletType::Boss ? false : DoesCollide(Bullet, &Game->Boss.BoundingBox);
        if (bHitsPlayer || bHitsBoss)
        {
            if (bHitsPlayer)
            {
                bool bDebugRedo = DoesCollide(Bullet, &Game->Player.BoundingBox);
                (void)bDebugRedo;
                ASSERT(Bullet.Type == BulletType::Boss);
                Game->Player.Hit(Game);
                NumBulletsBoss--;
            }
            else if (bHitsBoss)
            {
                bool bDebugRedo = DoesCollide(Bullet, &Game->Boss.BoundingBox);
                (void)bDebugRedo;
                ASSERT(Bullet.Type == BulletType::Player);
                Game->Boss.Hit(Game);
                NumBulletsPlayer--;
            }
            RenderEntityID idInactiveBullet = Bullet.ID;
            BulletRE->bVisible = false;
            ActiveBullets.Remove(Idx);
            InactiveBullets.Add(idInactiveBullet);
            Idx--;
        }
        else if (IsOffscreen(Bullet))
        {
            if (bDebugPrint)
            {
                Outf("[debug][BulletManager]: Despawned offscreen bullet\n");
                Outf("\tRenderEntityID: %d\n", Bullet.ID);
                Outf("\tType: %s\n", Bullet.Type == BulletType::Player ? "Player" : "Boss");
                Outf("\tPos: <%0.2f, %0.2f>\n", Bullet.Pos.X, Bullet.Pos.Y);
                Outf("\tDir: <%0.2f, %0.2f>\n", Bullet.Vel.X, Bullet.Vel.Y);
            }
            if (Bullet.Type == BulletType::Player) { NumBulletsPlayer--; }
            else { NumBulletsBoss--; }
            RenderEntityID idInactiveBullet = Bullet.ID;
            BulletRE->bVisible = false;
            ActiveBullets.Remove(Idx);
            InactiveBullets.Add(idInactiveBullet);
            Idx--;
        }
        else
        {
            float Scale = Bullet.Type == BulletType::Player ? PlayerBulletSize : BossBulletSize;
            BulletRE->World = m4f::Scale(Scale, Scale, 1.0f) * m4f::Trans(Bullet.Pos.X, Bullet.Pos.Y, 0.0f);
        }
    }
}

void Background::Init(UBGameImpl* Game)
{
    CurrColor = GetRandomColorDim();
    NextColor = GetRandomColorDim();
    LastSwitchTime = 0.0f;
    StepDurationSeconds = 2.0f;

    {
        VxMin BackgroundVerts[] =
        {
            { GlobalEngine->Width * -4.0f, GlobalEngine->Height * +4.0f, +0.99f, 1.0f },
            { GlobalEngine->Width * +4.0f, GlobalEngine->Height * +4.0f, +0.99f, 1.0f }, 
            { GlobalEngine->Width * -4.0f, GlobalEngine->Height * -4.0f, +0.99f, 1.0f }, 
            { GlobalEngine->Width * +4.0f, GlobalEngine->Height * -4.0f, +0.99f, 1.0f }, 
        };

        u32 BackgroundInds[] =
        {
            0, 1, 2,
            1, 3, 2
        };

        idBackgroundMesh = Game->Gfx.CreateMesh
        (
            sizeof(VxMin),
            ARRAY_SIZE(BackgroundVerts), BackgroundVerts,
            ARRAY_SIZE(BackgroundInds), BackgroundInds
        );
        ASSERT(idBackgroundMesh);
    }

    RenderEntity BackgroundRenderData = {};
    BackgroundRenderData.bVisible = true;
    BackgroundRenderData.World = m4f::Identity();
    BackgroundRenderData.Type = DrawType::Unicolor;
    BackgroundRenderData.idMesh = idBackgroundMesh;
    BackgroundRenderData.UnicolorState = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    idBackground = Game->Gfx.CreateEntity(BackgroundRenderData);
    ASSERT(idBackground);
}

void Background::Term(UBGameImpl* Game)
{
    Game->Gfx.DestroyEntity(idBackground);
    Game->Gfx.DestroyMesh(idBackgroundMesh);
}

void Background::Update(UBGameImpl* Game)
{
    RenderEntity* RE = Game->Gfx.GetEntity(idBackground);
    ASSERT(RE);
    /*
    { // Set cycle between predefined colors:
        constexpr v4f Colors[] = {
            { 1.0f, 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f, 0.1f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
        constexpr float StepDurationSeconds = 2.0f;
        constexpr size_t NumColors = ARRAY_SIZE(Colors);

        float CurrTime = (float)GlobalEngine->Clock->CurrTime;
        float Factor = (CurrTime / StepDurationSeconds) - (float)(int)(CurrTime / StepDurationSeconds);
        int StepNumber = (int)(CurrTime / StepDurationSeconds) % NumColors;
        RE->UnicolorState = { Lerp(Colors[StepNumber], Colors[(StepNumber + 1) % NumColors], Factor) };
    }
    */

    float CurrTime = (float)GlobalEngine->Clock->CurrTime;
    if (CurrTime - LastSwitchTime > StepDurationSeconds)
    {
        LastSwitchTime = CurrTime;
        CurrColor = NextColor;
        NextColor = GetRandomColorDim();
    }
    float Factor = (CurrTime - LastSwitchTime) / StepDurationSeconds;
    RE->UnicolorState = { Lerp(CurrColor, NextColor, Factor) };
}

bool UBGameImpl::Init()
{
    bool bResult = Gfx.Init((UBG_GfxT*)GlobalEngine->GfxState);

    BG.Init(this);
    BulletMgr.Init(this);
    Player.Init(this);
    Boss.Init(this);

    return bResult;
}

void UBGameImpl::Update()
{
    BG.Update(this);
    BulletMgr.Update(this);
    Player.Update(this);
    Boss.Update(this);
}

void UBGameImpl::Draw()
{
    Gfx.Entities.DrawAll(&Gfx);
}

bool UBGameImpl::Term()
{
    Player.Term(this);
    Boss.Term(this);
    BulletMgr.Term(this);
    BG.Term(this);

    bool bResult = Gfx.Term();
    return bResult;
}

