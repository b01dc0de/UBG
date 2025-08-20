#ifndef GAME_IMPL_H
#define GAME_IMPL_H

struct UBGameImpl;

// TODO: Implement a 'DefaultSpriteWorld'

struct VisualProgressBar
{
    static constexpr f32 fPadding = 2.0f;
    RenderEntityID idBG;
    RenderEntityID idBar;
    f32 Progress;
    v4f Color;
    v2f Pos;
    v2f Size;

    void Init(UBGameImpl* Game, f32 _Progress, v4f _Color, v2f _Pos, v2f _Size);
    void Term(UBGameImpl* Game);
    void Update(UBGameImpl* Game, f32 _Progress);
};

struct PlayerShip
{
    static constexpr bool bUseShipMesh = false;
    static constexpr f32 fMaxSpeed = 750.0f;
    static constexpr f32 MaxHealth = 100.0f;
    enum struct AimControls { Mouse, Keys, Auto };
    static constexpr AimControls AimBehavior = AimControls::Auto;

    TextureStateID idShipTexture;
    MeshStateID idShipMesh;
    RenderEntityID idShip;
    VisualProgressBar Healthbar;
    v2f Pos;
    v2f Vel;
    f32 Scale;
    f32 Angle;
    v2i Dir;
    f32 TurningProgress;
    f32 Health;
    AABB BoundingBox;

    void Hit(UBGameImpl* Game);
    void Init(UBGameImpl* Game);
    void Term(UBGameImpl* Game);
    void HandleInput(UBGameImpl* Game);
    void Update(UBGameImpl* Game);
};

struct BossShip
{
    static constexpr f32 MaxHealth = 100.0f;

    MeshStateID idShipMesh;
    RenderEntityID idShip;
    MeshStateID idBoundingBoxMesh;
    RenderEntityID idBoundingBox;
    VisualProgressBar Healthbar;
    AABB BoundingBox;
    f32 Scale;
    f32 Health;

    void Hit(UBGameImpl* Game);
    void Init(UBGameImpl* Game);
    void Term(UBGameImpl* Game);
    void Update(UBGameImpl* Game);
};

enum struct BulletType
{
    Player,
    Boss,
    Count
};

#define BULLETMGR_USE_INSTDRAW() (1)

struct PerBulletData
{
#if BULLETMGR_USE_INSTDRAW()
#else
    RenderEntityID ID;
#endif // BULLETMGR_USE_INSTDRAW()
    BulletType Type;
    v2f Pos;
    v2f Vel;
};

struct BulletManager
{
    static constexpr bool bDebugPrint = false;
    static constexpr int MaxBulletsPlayer = 256;
    static constexpr int MaxBulletsBoss = 256;
    static constexpr v4f PlayerBulletColor = { 0.8f, 0.8f, 0.8f, 0.8f };
    static constexpr v4f BossBulletColor = { 0.9f, 0.1f, 0.1f, 1.0f };
    static constexpr f32 PlayerBulletSize = 5.0f;
    static constexpr f32 BossBulletSize = 10.0f;

#if BULLETMGR_USE_INSTDRAW()
    RenderInstEntityID idInstBullets;
    DArray<InstRectColorData> BulletInstDrawData;
#else
    MeshStateID idBulletMesh;
    DArray<RenderEntityID> InactiveBullets;
#endif // BULLETMGR_USE_INSTDRAW()
    int NumBulletsPlayer;
    int NumBulletsBoss;
    DArray<PerBulletData> ActiveBullets;

    static bool DoesCollide(PerBulletData& Bullet, AABB* BoundingBox);
    static bool IsOffscreen(PerBulletData& Bullet);
    void NewBullet(UBGameImpl* Game, BulletType Type, v2f Pos, v2f Vel);
    void Init(UBGameImpl* Game);
    void Term(UBGameImpl* Game);
    void Update(UBGameImpl* Game);
};

struct Background
{
    MeshStateID idBackgroundMesh;
    RenderEntityID idBackground;
    v4f CurrColor;
    v4f NextColor;
    f32 LastSwitchTime;
    f32 StepDurationSeconds;

    void Init(UBGameImpl* Game);
    void Term(UBGameImpl* Game);
    void Update(UBGameImpl* Game);
};

struct UBGameImpl
{
    GfxSystem Gfx;

    Background BG;
    BulletManager BulletMgr;
    PlayerShip Player;
    BossShip Boss;

    bool Init();
    void Update();
    void Draw();
    bool Term();
};

#endif // GAME_IMPL_H

