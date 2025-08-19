#include "../UBG.h"
#include "Impl.h"

/*
    TODO:
        - PlayerShip
            - Implement different aim behaviors:
                - [X] Mouse
                - [ ] ArrowKeys
                - [ ] Auto
*/

void VisualProgressBar::Init(UBGameImpl* Game, f32 _Progress, v4f _Color, v2f _Pos, v2f _Size)
{
    Progress = Clamp(0.0f, 1.0f, _Progress);
    Color = _Color;
    Pos = _Pos;
    Size = _Size;

    m4f WorldBG = m4f::Scale(Size.X, Size.Y, 1.0f) * m4f::Trans(Pos.X, Pos.Y, +0.1f);
    m4f WorldBar = m4f::Scale((Size.X - fPadding * 2.0f) * Progress, Size.Y - fPadding * 2.0f, 1.0f) * m4f::Trans(Pos.X + fPadding, Pos.Y + fPadding, 0.0f);

    RenderEntity RenderData = RenderEntity::Default
    (
        WorldBG,
        DrawType::Unicolor,
        Game->Gfx.idQuadUnicolor
    );

    RenderData.UnicolorState = { v4f{ 0.75f, 0.75f, 0.75f, 1.0f } };
    idBG = Game->Gfx.CreateEntity(RenderData);
    ASSERT(idBG);

    RenderData.World = WorldBar;
    RenderData.UnicolorState = { Color };
    idBar = Game->Gfx.CreateEntity(RenderData);
    ASSERT(idBar);
}

void VisualProgressBar::Term(UBGameImpl* Game)
{
    Game->Gfx.DestroyEntity(idBG);
    Game->Gfx.DestroyEntity(idBar);
}

void VisualProgressBar::Update(UBGameImpl* Game, f32 _Progress)
{
    Progress = Clamp(0.0f, 1.0f, _Progress);
    m4f WorldBG = m4f::Scale(Size.X, Size.Y, 1.0f) * m4f::Trans(Pos.X, Pos.Y, +0.1f);
    m4f WorldBar = m4f::Scale((Size.X - fPadding * 2.0f) * Progress, Size.Y - fPadding * 2.0f, 1.0f) * m4f::Trans(Pos.X + fPadding, Pos.Y + fPadding, 0.0f);

    RenderEntity* DataBG = Game->Gfx.GetEntity(idBG);
    ASSERT(DataBG);
    DataBG->World = WorldBG;

    RenderEntity* DataBar = Game->Gfx.GetEntity(idBar);
    ASSERT(DataBar);
    DataBar->World = WorldBar;
}

void PlayerShip::Hit(UBGameImpl* Game)
{
    UNUSED_VAR(Game);
    Health -= 25.0f;
    if (Health <= 0.0f)
    {
        Term(Game);
        Init(Game);
    }
    else
    {
        Healthbar.Update(Game, Health / MaxHealth);
    }
}

void PlayerShip::Init(UBGameImpl* Game)
{
    Pos = { (float)GlobalEngine->Width * -0.25f, (float)GlobalEngine->Height * +0.25f };
    Vel = { 0.0f, 0.0f };
    Scale = 25.0f;
    Angle = 0.0f;
    Dir = { };
    TurningProgress = 0.0f;
    Health = MaxHealth;
    float HalfScale = Scale * 0.5f;
    BoundingBox =
    {
        { Pos.X - HalfScale, Pos.Y - HalfScale }, // Min
        { Pos.X + HalfScale, Pos.Y + HalfScale } // Max
    };
    constexpr v4f HealthbarColor{ 1.0f, 0.0f, 1.0f, 1.0f };
    constexpr v2f HealthbarSize{ 100.0f, 25.0f };
    constexpr v2f HealthbarTopLeftOffset{ HealthbarSize.X * +0.125f, HealthbarSize.Y * -1.5f };
    v2f HealthbarPos = Add({ (f32)GlobalEngine->Width * -0.5f, (f32)GlobalEngine->Height * +0.5f }, HealthbarTopLeftOffset);
    Healthbar.Init(Game, Health / MaxHealth, HealthbarColor, HealthbarPos, HealthbarSize);

    RenderEntity PlayerShipData = {};
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

        PlayerShipData = RenderEntity::Default(m4f::Identity(), DrawType::Unicolor, idShipMesh);
        PlayerShipData.UnicolorState = { v4f{ 1.0f, 1.0f, 1.0f, 1.0f } };
    }
    else
    {
        ImageT PlayerShipTextureImage = {};
        LoadBMPFile("Assets/player_ship.bmp", PlayerShipTextureImage);
        ASSERT(PlayerShipTextureImage.PxBuffer);
        idShipTexture = Game->Gfx.CreateTexture(&PlayerShipTextureImage);
        ASSERT(idShipTexture);
        delete[] PlayerShipTextureImage.PxBuffer;

        PlayerShipData = RenderEntity::Default(m4f::Identity(), DrawType::Texture, Game->Gfx.idQuadTexture);
        PlayerShipData.TextureState.idTexture = idShipTexture;
        PlayerShipData.TextureState.idSampler = Game->Gfx.idDefaultSampler;
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
    Healthbar.Term(Game);
}

void PlayerShip::HandleInput(UBGameImpl* Game)
{
    UNUSED_VAR(Game);

    f32 DeltaTime = (f32)GlobalEngine->Clock->LastFrameDuration;
    bool bKeyUp = GlobalEngine->Input->Keyboard.GetKey('W');
    bool bKeyLeft = GlobalEngine->Input->Keyboard.GetKey('A');
    bool bKeyDown = GlobalEngine->Input->Keyboard.GetKey('S');
    bool bKeyRight = GlobalEngine->Input->Keyboard.GetKey('D');
    // Handle movement input
    {
        static constexpr f32 fAcclFactor = 2.0f;
        v2f Acceleration = { };

        if (bKeyLeft) { Acceleration.X -= fAcclFactor; }
        if (bKeyRight) { Acceleration.X += fAcclFactor; }

        if (bKeyUp) { Acceleration.Y += fAcclFactor; }
        if (bKeyDown) { Acceleration.Y -= fAcclFactor; }

        // Keep diagonal movement the same
        if ((bKeyLeft || bKeyRight) && (bKeyUp || bKeyDown))
        {
            Acceleration *= fInvSqrt2;
        }

        static constexpr f32 fVelFactor = 250.0f;
        static constexpr f32 fFrictionFactor = 0.95f;
        Acceleration *= fVelFactor;
        Acceleration += Vel * -fFrictionFactor;

        // f(t) = (1/2)at^2 + vt + p;
        Pos = Acceleration * 0.5f * Square(DeltaTime) + Vel * DeltaTime + Pos;
        Vel += Acceleration * DeltaTime;
    }

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

    // Update ship angle
    switch (AimBehavior)
    {
        case AimControls::Mouse:
        {
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
            else
            {
                // Keep angle the same
            }
        } break;

        case AimControls::Keys:
        {
            static constexpr f32 TurnSpeed = fPI * (1.0f / 12.0f);

            bool bArrowUp = GlobalEngine->Input->Keyboard.GetKey(VK_UP);
            bool bArrowDown = GlobalEngine->Input->Keyboard.GetKey(VK_DOWN);
            bool bArrowLeft = GlobalEngine->Input->Keyboard.GetKey(VK_LEFT);
            bool bArrowRight = GlobalEngine->Input->Keyboard.GetKey(VK_RIGHT);

            // Get new dir from input
            v2i InputDir = {};
            if (bArrowUp != bArrowDown) { InputDir.Y = bArrowUp ? -1 : +1; }
            if (bArrowLeft != bArrowRight) { InputDir.X = bArrowLeft ? -1 : +1; }

            // If there is new input AND new dir is different
            //  - Set new dir
            //  - Start turning to new dir
            if ((InputDir.X || InputDir.Y) && InputDir != Dir)
            {
                Dir = InputDir;
                TurningProgress = 0.0f;
            }
            else
            {
                TurningProgress = Clamp(0.0f, 1.0f, TurningProgress + TurnSpeed * DeltaTime);
            }
            float AngleDir = atan2f((float)Dir.Y, (float)Dir.X);
            Angle = Lerp(Angle, AngleDir, TurningProgress);
        } break;

        case AimControls::Auto:
        {
            Angle = atan2f(Pos.Y, -Pos.X);
        } break;

        default:
        {
            ASSERT(false);
        } break;
    }

    // Spawn bullets
    static f32 LastBulletSpawn = 0.0f;
    static constexpr f32 SecondsPerBullet = 0.25f;
    static constexpr f32 Speed = 100.0f;
    f32 CurrTime = (f32)GlobalEngine->Clock->CurrTime;
    bool bShoot = false;
    switch (AimBehavior)
    {
        case AimControls::Mouse:
        {
            bShoot = GlobalEngine->Input->Mouse.LeftButton;
        } break;

        case AimControls::Keys:
        case AimControls::Auto:
        {
            bShoot = true;
        } break;

        default:
        {
            ASSERT(false);
        } break;
    }

    if (bShoot && ((CurrTime - LastBulletSpawn) > SecondsPerBullet))
    {
        v2f BulletVel = { cosf(Angle) * Speed, -sinf(Angle) * Speed };
        v2f BulletPos = Pos + (Norm(BulletVel) * Scale) * 0.75f;
        Game->BulletMgr.NewBullet(Game, BulletType::Player, BulletPos, BulletVel);
        LastBulletSpawn = CurrTime;
    }
}

void PlayerShip::Update(UBGameImpl* Game)
{
    HandleInput(Game);

    { // Update world:
        RenderEntity* pRent = Game->Gfx.GetEntity(idShip);
        ASSERT(pRent);
        pRent->World = m4f::Scale(Scale, Scale, 1.0f) * m4f::RotZ(Angle) * m4f::Trans(Pos.X, Pos.Y, 0.0f);
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
    else
    {
        Healthbar.Update(Game, Health / MaxHealth);
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
    Health = MaxHealth;
    constexpr v4f HealthbarColor{ 1.0f, 0.0f, 0.0f, 1.0f };
    constexpr v2f HealthbarSize{ 100.0f, 25.0f };
    constexpr v2f HealthbarTopRightOffset{ HealthbarSize.X * -1.125f, HealthbarSize.Y * -1.5f };
    v2f HealthbarPos = Add({ (f32)GlobalEngine->Width * +0.5f, (f32)GlobalEngine->Height * +0.5f }, HealthbarTopRightOffset);
    Healthbar.Init(Game, Health / MaxHealth, HealthbarColor, HealthbarPos, HealthbarSize);

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

        RenderEntity BoundingBoxRenderData = RenderEntity::Default(m4f::Scale(Scale, Scale, 1.0f), DrawType::Unicolor, idBoundingBoxMesh);
        BoundingBoxRenderData.UnicolorState = { { 1.0f, 1.0f, 0.0f, 1.0f } };
        idBoundingBox = Game->Gfx.CreateEntity(BoundingBoxRenderData);
    }

    BoundingBox.Min.X *= Scale;
    BoundingBox.Min.Y *= Scale;
    BoundingBox.Max.X *= Scale;
    BoundingBox.Max.Y *= Scale;
    RenderEntity BossShipData = RenderEntity::Default(m4f::Scale(Scale, Scale, 1.0), DrawType::Unicolor, idShipMesh);
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
            RenderEntity REBulletData = RenderEntity::Default(m4f::Identity(), DrawType::Unicolor, idBulletMesh);
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

    RenderEntity BackgroundRenderData = RenderEntity::Default(m4f::Identity(), DrawType::Unicolor, idBackgroundMesh);
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

