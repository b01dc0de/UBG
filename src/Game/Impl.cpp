#include "../UBG.h"
#include "Impl.h"

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
    RenderData.StageIndex = (u8)DrawStage::UI;

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
    PlayerShipData.StageIndex = (u8)DrawStage::MAIN_GAMEPLAY;
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
    Healthbar.Term(Game);
    if (bUseShipMesh) { Game->Gfx.DestroyMesh(idShipMesh); }
    else { Game->Gfx.DestroyTexture(idShipTexture); }
    Game->Gfx.DestroyEntity(idShip);
}

void PlayerShip::HandleInput(UBGameImpl* Game)
{
    UNUSED_VAR(Game);

    f32 DeltaTime = (f32)GlobalEngine->Clock->LastFrameDuration;
    bool bKeyUp = GlobalEngine->Input->Keyboard.GetKey(UBG_KEY_W);
    bool bKeyLeft = GlobalEngine->Input->Keyboard.GetKey(UBG_KEY_A);
    bool bKeyDown = GlobalEngine->Input->Keyboard.GetKey(UBG_KEY_S);
    bool bKeyRight = GlobalEngine->Input->Keyboard.GetKey(UBG_KEY_D);
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
    float MinX = HalfScale - HalfWidth;
    float MinY = HalfScale - HalfHeight;
    float MaxX = GlobalEngine->Width - HalfScale - HalfWidth;
    float MaxY = GlobalEngine->Height - HalfScale - HalfHeight;
    bool bShouldClampX = (Pos.X < MinX || MaxX < Pos.X);
    bool bShouldClampY = (Pos.Y < MinY || MaxY < Pos.Y);
    if (bShouldClampX) { Vel.X = 0.0f; }
    if (bShouldClampY) { Vel.Y = 0.0f; }
    Pos.X = Clamp(MinX, MaxX, Pos.X);
    Pos.Y = Clamp(MinY, MaxY, Pos.Y);

    static constexpr bool bUseAccurateBB = true;
    if (bUseAccurateBB)
    {
        v2f Points[] = {
            { -0.5f, +0.5f },
            { +0.5f, +0.5f },
            { -0.5f, -0.5f },
            { +0.5f, -0.5f }
        };

        f32 fCos = cosf(Angle);
        f32 fSin = sinf(Angle);
        v2f Min = Pos;
        v2f Max = Pos;

        for (int Idx = 0; Idx < ARRAY_SIZE(Points); Idx++)
        {
            v2f RotPoint = {
                (fCos * Points[Idx].X - fSin * Points[Idx].Y) * Scale + Pos.X,
                (fSin * Points[Idx].X + fCos * Points[Idx].Y) * Scale + Pos.Y
            };
            if (RotPoint.X < Min.X) { Min.X = RotPoint.X; }
            if (RotPoint.Y < Min.Y) { Min.Y = RotPoint.Y; }
            if (RotPoint.X > Max.X) { Max.X = RotPoint.X; }
            if (RotPoint.Y > Max.Y) { Max.Y = RotPoint.Y; }
        }
        BoundingBox = { Min, Max };
    }
    else
    {
        BoundingBox =
        {
            { Pos.X - HalfScale, Pos.Y - HalfScale }, // Min
            { Pos.X + HalfScale, Pos.Y + HalfScale } // Max
        };
    }
    Game->DbgVis.BoundingBoxDraws.Add({ { BoundingBox.Min, BoundingBox.Max - BoundingBox.Min }, v4f{1.0f, 1.0f, 1.0f, 1.0f} });

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
            else { } // Keep angle the same
        } break;

        case AimControls::Keys:
        {
            static constexpr f32 TurnSpeed = fPI * (1.0f / 12.0f);

            bool bArrowUp = GlobalEngine->Input->Keyboard.GetKey(UBG_KEY_ARROW_UP);
            bool bArrowDown = GlobalEngine->Input->Keyboard.GetKey(UBG_KEY_ARROW_DOWN);
            bool bArrowLeft = GlobalEngine->Input->Keyboard.GetKey(UBG_KEY_ARROW_LEFT);
            bool bArrowRight = GlobalEngine->Input->Keyboard.GetKey(UBG_KEY_ARROW_RIGHT);

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
            //BossInds[BaseIdx + 1] = TriIdx == BossNumTris - 1 ? 1 : TriIdx + 2;
            BossInds[BaseIdx + 1] = TriIdx + 1;
            BossInds[BaseIdx + 2] = TriIdx == BossNumTris - 1 ? 1 : TriIdx + 2;
        }
        idShipMesh = Game->Gfx.CreateMesh(sizeof(VxMin), BossNumVerts, BossVerts, BossNumInds, BossInds);
        ASSERT(idShipMesh);
    }

    BoundingBox.Min.X *= Scale;
    BoundingBox.Min.Y *= Scale;
    BoundingBox.Max.X *= Scale;
    BoundingBox.Max.Y *= Scale;
    RenderEntity BossShipData = RenderEntity::Default(m4f::Scale(Scale, Scale, 1.0), DrawType::Unicolor, idShipMesh);
    BossShipData.UnicolorState.Color = ColorScheme::BossShip;
    BossShipData.StageIndex = (u8)DrawStage::MAIN_GAMEPLAY;
    idShip = Game->Gfx.CreateEntity(BossShipData);
    ASSERT(idShip);
}

void BossShip::Term(UBGameImpl* Game)
{
    Healthbar.Term(Game);
    Game->Gfx.DestroyEntity(idShip);
    Game->Gfx.DestroyMesh(idShipMesh);
}

void BossShip::Update(UBGameImpl* Game)
{
    f32 CurrTime = (f32)GlobalEngine->Clock->CurrTime;

    if ((CurrTime - LastBulletSpawn) > SecondsPerBullet)
    {
        switch (Style)
        {
            case AttackStyle::None:
            {
            } break;

            case AttackStyle::Follow:
            {
                f32 PlayerAngle = atan2f(Game->Player.Pos.Y, Game->Player.Pos.X);
                v2f Dir = { cosf(PlayerAngle) * BulletSpeed, sinf(PlayerAngle) * BulletSpeed };
                Game->BulletMgr.NewBullet(Game, BulletType::Boss, v2f{ 0.0f, 0.0f }, Dir);
            } break;

            case AttackStyle::Clockwise:
            {
                static f32 SpawnDir = 0.0f;
                static f32 TurnSpeed = SecondsPerBullet * fPI;
                v2f Dir = { cosf(SpawnDir) * BulletSpeed, sinf(SpawnDir) * BulletSpeed };
                SpawnDir -= TurnSpeed;
                Game->BulletMgr.NewBullet(Game, BulletType::Boss, v2f{ 0.0f, 0.0f }, Dir);
            } break;

            case AttackStyle::Random:
            {
                f32 RandomAngle = GetRandomFloatNorm() * fTAU;
                v2f Dir = { cosf(RandomAngle) * BulletSpeed, sinf(RandomAngle) * BulletSpeed };
                Game->BulletMgr.NewBullet(Game, BulletType::Boss, v2f{ 0.0f, 0.0f }, Dir);
            } break;

            default:
            {
                ASSERT(false);
            } break;
        }

        LastBulletSpawn = CurrTime;
    }

    Game->DbgVis.BoundingBoxDraws.Add({ { BoundingBox.Min, BoundingBox.Max - BoundingBox.Min }, ColorScheme::BossShip });
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
    UNUSED_VAR(Game);

    // TODO: Do we care about enforcing a strict limit on NumBullets?
    if (Type == BulletType::Player && NumBulletsPlayer < MaxBulletsPlayer)
    {
        PerBulletData NewBullet = { /*Type,*/ Pos, Vel};
        ActivePlayerBullets.Add(NewBullet);
    }
    else if (Type == BulletType::Boss && NumBulletsBoss < MaxBulletsBoss)
    {
        PerBulletData NewBullet = { /*Type,*/ Pos, Vel};
        ActiveBossBullets.Add(NewBullet);
    }
}

void BulletManager::Init(UBGameImpl* Game)
{
    BulletDraws.Reserve(MeshInstStateT::DefaultMaxInstCount);
    ActivePlayerBullets.Reserve(MaxBulletsPlayer);
    ActiveBossBullets.Reserve(MaxBulletsBoss);

    RenderInstEntity REInstData = {};
    REInstData.StageIndex = (u8)DrawStage::MAIN_GAMEPLAY;
    REInstData.Type = DrawInstType::Color;
    // Bullet mesh:
    {
        constexpr u32 NumPoints = BulletMeshResolution;
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
            Inds[BaseIdx + 1] = TriIdx + 1;
            Inds[BaseIdx + 2] = TriIdx == NumTris - 1 ? 1 : TriIdx + 2;
        }

        idInstBulletMesh = Game->Gfx.CreateMeshInst
        (
            sizeof(VxMin),
            sizeof(InstRectColorData),
            MeshInstStateT::DefaultMaxInstCount,
            NumVerts, Verts,
            NumInds, Inds
        );
        ASSERT(idInstBulletMesh);
        REInstData.idMesh = idInstBulletMesh;
    }
    idInstBullets = Game->Gfx.CreateEntityInst(REInstData);
}

void BulletManager::Term(UBGameImpl* Game)
{
    RenderInstEntity* InstBulletData = Game->Gfx.GetEntityInst(idInstBullets);
    ASSERT(InstBulletData);
    ASSERT(idInstBulletMesh);
    Game->Gfx.DestroyMeshInst(idInstBulletMesh);
    Game->Gfx.DestroyEntityInst(idInstBullets);
    ActivePlayerBullets.Term();
    ActiveBossBullets.Term();
}

void BulletManager::Update(UBGameImpl* Game)
{
    BulletDraws.Clear();
    for (size_t Idx = 0; Idx < ActivePlayerBullets.Num; Idx++)
    {
        PerBulletData& Bullet = ActivePlayerBullets[Idx];
        f32 dt = (f32)GlobalEngine->Clock->LastFrameDuration;
        v2f AdjVel = { Bullet.Vel.X * dt, Bullet.Vel.Y * dt };
        Bullet.Pos = Bullet.Pos + AdjVel;

        SphereBB BulletBB = { Bullet.Pos, PlayerBulletSize};
        bool bHitsBoss = Collision::Check(&Game->Boss.BoundingBox, &BulletBB);
        if (bHitsBoss)
        {
            //ASSERT(Bullet.Type == BulletType::Player);
            Game->Boss.Hit(Game);
            NumBulletsPlayer--;
            ActivePlayerBullets.Remove(Idx);
            Idx--;
        }
        else if (IsOffscreen(Bullet))
        {
            if (bDebugPrint)
            {
                Outf("[debug][BulletManager]: Despawned offscreen bullet\n");
                Outf("\tType: Player\n\tPos: <%0.2f, %0.2f>\n", Bullet.Pos.X, Bullet.Pos.Y);
                Outf("\tDir: <%0.2f, %0.2f>\n", Bullet.Vel.X, Bullet.Vel.Y);
            }
            ActivePlayerBullets.Remove(Idx);
            Idx--;
        }
        else
        {
            static constexpr bool bDrawBulletOutline = true;
            float Scale = PlayerBulletSize;
            v4f Color = ColorScheme::PlayerBullets;
            InstRectColorData BulletDrawData = {};
            BulletDrawData.Rect = { Bullet.Pos, v2f{Scale, Scale} };
            BulletDrawData.Color = Color;
            BulletDraws.Add(BulletDrawData);
            if (bDrawBulletOutline)
            {
                float fOutlineSize = Scale * 0.2f;
                BulletDrawData.Rect = { Bullet.Pos, v2f{Scale + fOutlineSize, Scale + fOutlineSize} };
                BulletDrawData.Color = ColorScheme::BulletOutline;
                BulletDraws.Add(BulletDrawData);
            }
        }
    }

    for (size_t Idx = 0; Idx < ActiveBossBullets.Num; Idx++)
    {
        PerBulletData& Bullet = ActiveBossBullets[Idx];
        f32 dt = (f32)GlobalEngine->Clock->LastFrameDuration;
        v2f AdjVel = { Bullet.Vel.X * dt, Bullet.Vel.Y * dt };
        Bullet.Pos = Bullet.Pos + AdjVel;

        SphereBB BulletBB = { Bullet.Pos, BossBulletSize };
        bool bHitsPlayer = Collision::Check(&Game->Player.BoundingBox, &BulletBB);
        if (bHitsPlayer)
        {
            //ASSERT(Bullet.Type == BulletType::Boss);
            Game->Player.Hit(Game);
            NumBulletsBoss--;
            ActiveBossBullets.Remove(Idx);
            Idx--;
        }
        else if (IsOffscreen(Bullet))
        {
            if (bDebugPrint)
            {
                Outf("[debug][BulletManager]: Despawned offscreen bullet\n");
                Outf("\tType: Boss\n\tPos: <%0.2f, %0.2f>\n", Bullet.Pos.X, Bullet.Pos.Y);
                Outf("\tDir: <%0.2f, %0.2f>\n", Bullet.Vel.X, Bullet.Vel.Y);
            }
            ActiveBossBullets.Remove(Idx);
            Idx--;
        }
        else
        {
            static constexpr bool bDrawBulletOutline = true;
            float Scale = BossBulletSize;
            v4f Color = ColorScheme::BossBullets;
            InstRectColorData BulletDrawData = {};
            BulletDrawData.Rect = { Bullet.Pos, v2f{Scale, Scale} };
            BulletDrawData.Color = Color;
            BulletDraws.Add(BulletDrawData);
            if (bDrawBulletOutline)
            {
                float fOutlineSize = Scale * 0.2f;
                BulletDrawData.Rect = { Bullet.Pos, v2f{Scale + fOutlineSize, Scale + fOutlineSize} };
                BulletDrawData.Color = ColorScheme::BulletOutline;
                BulletDraws.Add(BulletDrawData);
            }
        }
    }

    // NOTE: For _safety reasons_ right now: we require explicitly setting idInst's array num / array data every frame
    if (BulletDraws.Num)
    {
        RenderInstEntity* InstRE = Game->Gfx.GetEntityInst(idInstBullets);
        ASSERT(InstRE);
        InstRE->NumInst = BulletDraws.Num;
        InstRE->pInstData = BulletDraws.Data;
    }
}

void DebugVisualizer::Init(UBGameImpl* Game)
{
    BoundingBoxDraws.Reserve(MeshInstStateT::DefaultMaxInstCount);

    RenderInstEntity InstData = {};
    InstData.bWireframe = true;
    InstData.StageIndex = (u8)DrawStage::DEBUG;
    InstData.Type = DrawInstType::Color;
    InstData.idMesh = Game->Gfx.idInstRectColorLines;
    idInstBBs = Game->Gfx.CreateEntityInst(InstData);
    ASSERT(idInstBBs);
}

void DebugVisualizer::Term(UBGameImpl* Game)
{
    Game->Gfx.DestroyEntityInst(idInstBBs);

    BoundingBoxDraws.Term();
}

void DebugVisualizer::Update(UBGameImpl* Game)
{
    RenderInstEntity* pData = Game->Gfx.GetEntityInst(idInstBBs);
    ASSERT(pData)

    if (pData->bVisible && BoundingBoxDraws.Num)
    {
        pData->NumInst = BoundingBoxDraws.Num;
        pData->pInstData = BoundingBoxDraws.Data;
    }

    BoundingBoxDraws.Clear();
}

void Background::Init(UBGameImpl* Game)
{
    CurrColor = GetRandomColorDim();
    NextColor = GetRandomColorDim();
    LastSwitchTime = 0.0f;
    StepDurationSeconds = 2.0f;

    // Flat background
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
            0, 2, 1,
            1, 2, 3
        };

        idBackgroundMesh = Game->Gfx.CreateMesh
        (
            sizeof(VxMin),
            ARRAY_SIZE(BackgroundVerts), BackgroundVerts,
            ARRAY_SIZE(BackgroundInds), BackgroundInds
        );
        ASSERT(idBackgroundMesh);

        RenderEntity BackgroundRenderData = RenderEntity::Default(m4f::Identity(), DrawType::Unicolor, idBackgroundMesh);
        BackgroundRenderData.StageIndex = (u8)DrawStage::BG_PURE;
        BackgroundRenderData.UnicolorState = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        idBackground = Game->Gfx.CreateEntity(BackgroundRenderData);
        ASSERT(idBackground);
    }

    // Background grid:
    {
        static constexpr bool bUseLines = true;

        NumCellsX = 16;
        NumCellsY = 9;

        size_t NumVertsX = NumCellsX + 1;
        size_t NumVertsY = NumCellsY + 1;
        NumVerts = NumVertsX * NumVertsY;
        GridMeshVerts = new VxMin[NumVerts];

        float fWidth = (float)GlobalEngine->Width;
        float fHeight = (float)GlobalEngine->Height;
        float HalfWidth = fWidth * 0.5f;
        float HalfHeight = fHeight * 0.5f;
        for (size_t X = 0; X < NumVertsX; X++)
        {
            for (size_t Y = 0; Y < NumVertsY; Y++)
            {
                // Within range of normalized coordinates [-.5, +.5]
                GridMeshVerts[Y * NumVertsX + X] = {
                    HalfWidth - ((float)X / (float)(NumVertsX - 1)) * fWidth,
                    HalfHeight - ((float)Y / (float)(NumVertsY - 1)) * fHeight,
                    0.0f,
                        1.0f
                };
            }
        }

        size_t NumInds = 0;
        u32* GridMeshInds = nullptr;
        if (bUseLines)
        {
            size_t NumLines = NumCellsX * NumCellsY * 4;
            NumInds = NumLines * 2;
            GridMeshInds = new u32[NumInds];

            size_t IndexWrite = 0;
            for (size_t X = 0; X < NumCellsX; X++)
            {
                for (size_t Y = 0; Y < NumCellsY; Y++)
                {
                    u32 TopLeftIndex = (u32)(Y * NumVertsX + X);
                    u32 TopRightIndex = (u32)(TopLeftIndex + 1);
                    u32 BotLeftIndex = (u32)(TopLeftIndex + NumVertsX);
                    u32 BotRightIndex = (u32)(BotLeftIndex + 1);

                    GridMeshInds[IndexWrite + 0] = TopLeftIndex;
                    GridMeshInds[IndexWrite + 1] = TopRightIndex;

                    GridMeshInds[IndexWrite + 2] = TopLeftIndex;
                    GridMeshInds[IndexWrite + 3] = BotLeftIndex;

                    GridMeshInds[IndexWrite + 4] = BotLeftIndex;
                    GridMeshInds[IndexWrite + 5] = BotRightIndex;

                    GridMeshInds[IndexWrite + 6] = TopRightIndex;
                    GridMeshInds[IndexWrite + 7] = BotRightIndex;

                    IndexWrite += 8;
                }
            }
            ASSERT(IndexWrite == NumInds);

            idGridMesh = Game->Gfx.CreateMesh
            (
                sizeof(VxMin),
                NumVerts,
                GridMeshVerts,
                NumInds,
                GridMeshInds,
                D3D11_PRIMITIVE_TOPOLOGY_LINELIST
            );
            ASSERT(idGridMesh);
        }
        else
        {
            size_t NumTris = NumCellsX * NumCellsY * 2;
            NumInds = NumTris * 3;
            GridMeshInds = new u32[NumInds];

            size_t IndexWrite = 0;
            for (size_t X = 0; X < NumCellsX; X++)
            {
                for (size_t Y = 0; Y < NumCellsY; Y++)
                {
                    u32 TopLeftIndex = (u32)(Y * NumVertsX + X);
                    u32 TopRightIndex = (u32)(TopLeftIndex + 1);
                    u32 BotLeftIndex = (u32)(TopLeftIndex + NumVertsX);
                    u32 BotRightIndex = (u32)(BotLeftIndex + 1);

                    GridMeshInds[IndexWrite + 0] = TopLeftIndex;
                    GridMeshInds[IndexWrite + 1] = BotLeftIndex;
                    GridMeshInds[IndexWrite + 2] = TopRightIndex;
                    IndexWrite += 3;

                    GridMeshInds[IndexWrite + 0] = TopRightIndex;
                    GridMeshInds[IndexWrite + 1] = BotLeftIndex;
                    GridMeshInds[IndexWrite + 2] = BotRightIndex;
                    IndexWrite += 3;
                }
            }
            ASSERT(IndexWrite == NumInds);

            idGridMesh = Game->Gfx.CreateMesh
            (
                sizeof(VxMin),
                NumVerts,
                GridMeshVerts,
                NumInds,
                GridMeshInds
            );
            ASSERT(idGridMesh);
        }

        ASSERT(idGridMesh);
        delete[] GridMeshInds;
        RenderEntity GridMeshData = RenderEntity::Default(m4f::Identity(), DrawType::Unicolor, idGridMesh);
        GridMeshData.bWireframe = true;
        GridMeshData.StageIndex = (u8)DrawStage::BG_REAL;
        GridMeshData.World = m4f::Trans(0.0f, 0.0f, +0.75f);
        GridMeshData.UnicolorState = { ColorScheme::BackgroundGrid };
        idGrid = Game->Gfx.CreateEntity(GridMeshData);
        ASSERT(idGrid);
    }
}

void Background::Term(UBGameImpl* Game)
{
    delete[] GridMeshVerts;
    Game->Gfx.DestroyEntity(idBackground);
    Game->Gfx.DestroyMesh(idBackgroundMesh);
    Game->Gfx.DestroyEntity(idGrid);
    Game->Gfx.DestroyMesh(idGridMesh);
}

void Background::Update(UBGameImpl* Game)
{
    RenderEntity* RE = Game->Gfx.GetEntity(idBackground);
    ASSERT(RE);

    float CurrTime = (float)GlobalEngine->Clock->CurrTime;

    switch (Mode)
    {
        case ColorMode::Black:
        {
            RE->UnicolorState = { v4f{0.0f, 0.0f, 0.0f, 1.0f } };
        } break;

        case ColorMode::RandomDim:
        {
            if (CurrTime - LastSwitchTime > StepDurationSeconds)
            {
                LastSwitchTime = CurrTime;
                CurrColor = NextColor;
                NextColor = GetRandomColorDim();
            }
            float Factor = (CurrTime - LastSwitchTime) / StepDurationSeconds;
            RE->UnicolorState = { Lerp(CurrColor, NextColor, Factor) };
        } break;

        case ColorMode::EmulateSound:
        {
            f32 MinIntensity = 0.075f;
            f32 MaxIntensity = 0.25f;
            v4f SoundColor = {};

            auto GetFactor = [CurrTime](f32 BeatTime)
            {
                f32 Resolution = BeatTime / 10.0f;
                f32 BeatFactor = fmodf(CurrTime, BeatTime);
                f32 Dist = Min(BeatFactor, BeatTime - BeatFactor);

                f32 Result = 0.0f;
                if (Dist < Resolution)
                {
                    Result = (Resolution - Dist) / Resolution;
                }
                return Result;
            };

            f32 BaseBeatTime = 60.0f / BeatsPerMinute;

            f32 RedFactor = Lerp(MinIntensity, MaxIntensity, GetFactor(BaseBeatTime));
            f32 GreenFactor = Lerp(MinIntensity, MaxIntensity, GetFactor(BaseBeatTime * 2.0f));
            f32 BlueFactor = Lerp(MinIntensity, MaxIntensity, GetFactor(BaseBeatTime * 4.0f));

            SoundColor = { RedFactor, GreenFactor, BlueFactor, 1.0f };

            RE->UnicolorState = { SoundColor };
        } break;

        default:
        {
            ASSERT(false);
        } break;
    }
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

    constexpr bool bUpdateGrid = true;
    if (bUpdateGrid)
    {
        constexpr float SecondsPerUpdate = 1.0f / 15.0f;
        static float LastUpdate = 0.0f;

        if ((CurrTime - LastUpdate) > SecondsPerUpdate)
        {
            LastUpdate = CurrTime;

            size_t NumVertsX = NumCellsX + 1;
            size_t NumVertsY = NumCellsY + 1;
            float fWidth = (float)GlobalEngine->Width;
            float fHeight = (float)GlobalEngine->Height;
            float HalfWidth = fWidth * 0.5f;
            float HalfHeight = fHeight * 0.5f;
            float CellSizeX = 1.0f / (float)(NumVertsX - 1) * fWidth;
            float CellSizeY = 1.0f / (float)(NumVertsY - 1) * fHeight;
            float MaxVarianceX = CellSizeX * 0.25f;
            float MaxVarianceY = CellSizeY * 0.25f;
            for (size_t X = 0; X < NumVertsX; X++)
            {
                for (size_t Y = 0; Y < NumVertsY; Y++)
                {
                    float OriginalX = HalfWidth - ((float)X / (float)(NumVertsX - 1)) * fWidth;
                    float OriginalY = HalfHeight - ((float)Y / (float)(NumVertsY - 1)) * fHeight;

                    size_t VxIdx = Y * NumVertsX + X;
                    float CurrX = GridMeshVerts[VxIdx].Pos.X;
                    float CurrY = GridMeshVerts[VxIdx].Pos.Y;

                    float DiffX = (GetRandomFloatNorm() - 0.5f) * MaxVarianceX;
                    float DiffY = (GetRandomFloatNorm() - 0.5f) * MaxVarianceY;

                    float MinX = OriginalX - MaxVarianceX;
                    float MaxX = OriginalX + MaxVarianceX;
                    float MinY = OriginalY - MaxVarianceY;
                    float MaxY = OriginalY + MaxVarianceY;
                    float NewX = Clamp(MinX, MaxX, CurrX + DiffX);
                    float NewY = Clamp(MinY, MaxY, CurrY + DiffY);

                    GridMeshVerts[VxIdx] = { NewX, NewY, 0.0f, 1.0f };
                }
            }

            MeshStateT* GridMesh = Game->Gfx.GetMesh(idGridMesh);
            ASSERT(GridMesh);

            Game->Gfx.Backend->Context->UpdateSubresource(
                GridMesh->VxBuffer, 0, nullptr,
                GridMeshVerts, (u32)(sizeof(VxMin) * NumVerts), 0
            );
        }
    }
}

bool UBGameImpl::Init()
{
    bool bResult = Gfx.Init((UBG_GfxT*)GlobalEngine->GfxState);
    ASSERT(bResult);

    BG.Init(this);
    DbgVis.Init(this);
    BulletMgr.Init(this);
    Player.Init(this);
    Boss.Init(this);

    return bResult;
}

void UBGameImpl::Update()
{
    BG.Update(this);
    DbgVis.Update(this);
    BulletMgr.Update(this);
    Player.Update(this);
    Boss.Update(this);
}

void UBGameImpl::Draw()
{
    Gfx.Draw();
}

bool UBGameImpl::Term()
{
    Boss.Term(this);
    Player.Term(this);
    BulletMgr.Term(this);
    DbgVis.Term(this);
    BG.Term(this);

    bool bResult = Gfx.Term();
    return bResult;
}

