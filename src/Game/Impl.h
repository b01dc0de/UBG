#ifndef GAME_IMPL_H
#define GAME_IMPL_H

struct UBGameImpl;

// ENGINE TODOS:
// TODO: Implement a 'DefaultSpriteWorld'
//      - Decide on final origin placement (middle of screen OR bottom left? (top left, even?))

// GAME TODOS:
// TODO: Implement a real background
//      - 3D meshes in the background to give illusion of flying around/in space
//      - Ideas: scrolling texture, Resogun-like center cylinder 'world', grid-like mesh that bends and is effected by gameplay (water-like, even)
// TODO: Implement a particle emitter for effects
// TODO: Implement boss ship behavior
//      - Different attack styles
//      - Ability to switch between different patterns
//      - Spawning multiple bullets at a time in different patterns
// TODO: Implement different PlayerShip stats/config
//      - Bullet size/speed
//      - Ship size/speed
//      - Bullet types (3-wide, different bullet effects/qualities)
// TODO: Implement a basic GameObject interface
//      - I've tried to avoid this for now, but it seems necessary at this point

/*
    - NOTE: Currently, here's the few rendering 'stages' that we use
        - Pure background
        - TODO: Real background (i.e. 3d meshes in the background of the level)
        - Ships (player + enemy)
        - TODO: Particles/Effects
        - Bullets
        - UI
    - The easy way of distinguishing/enforcing these can be:
        - Every (2D) mesh lives on the XY plane (Z == 0)
        - Each mesh belongs to a given layer
        - Each layer corresponds to a different Z transpose for that layer
            and we apply that to each Entity's World matrix
        - We would still care about dividing the background world from the main gameplay
            (rendering background world completely first BEFORE everything else)
    - The more complex way would be having some sort of formal layer system:
        - Each layer would need to know about its current RenderEntity's
        - This could potentially also be used to batch common graphics state
        - Depending on Alpha/BlendState setup we wouldn't need to care about Z (although we should anyway)
        - And simply just render entities from the furthest back layer to the front
        - Requires a lot more work than the above solution
    - A less complex layer system:
        - We define an enum (or integer range, whatever) of distinct layers
            - Each RenderEntity/RenderEntityInst has a set layer value
            - At Gfx::Draw:
                - Compile a list of RenderEntity, Layer pairs
                - For each layer (starting at Layer 0 through Layer N):
                    - Draw all entities / entitiesinst
        - Other thoughts about this:
            - It's very unlikely that a RenderEntity will ever need to change layers (without its ID being recycled, etc.)
            - This could be expanded into include more layer information, for example:
                - Camera info (2D vs 3D, etc.)
                - DrawType
                - Alpha / Blend state
                - Wireframe / etc.
*/

struct ColorScheme
{
    static constexpr v4f BossShip{ 1.0f, 0.0f, 0.0f, 1.0f };
    static constexpr v4f BossBullets{ 0.9f, 0.1f, 0.1f, 1.0f };
    static constexpr v4f PlayerBullets{ 0.1f, 0.1f, 0.1f, 1.0f };
    static constexpr v4f BulletOutline{ 1.0f, 1.0f, 1.0f, 0.8f };
    static constexpr v4f BackgroundGrid{57.0f / 255.0f, 66.0f / 255.0f, 69.0f / 255.0f, 1.0f};
};

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
    enum struct AttackStyle { None, Follow, Clockwise, Random };

    static constexpr f32 MaxHealth = 100.0f;

    MeshStateID idShipMesh;
    RenderEntityID idShip;
    VisualProgressBar Healthbar;
    AABB BoundingBox;
    f32 Scale;
    f32 Health;
    f32 LastBulletSpawn = 0.0f;
    static constexpr f32 SecondsPerBullet = 0.25f;
    static constexpr f32 BulletSpeed = 100.0f;
    AttackStyle Style = AttackStyle::Follow;

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

struct PerBulletData
{
    BulletType Type;
    v2f Pos;
    v2f Vel;
};

struct BulletManager
{
    static constexpr bool bDebugPrint = false;
    static constexpr u32 BulletMeshResolution = 16;
    static constexpr s32 MaxBulletsPlayer = 256;
    static constexpr s32 MaxBulletsBoss = 256;
    static constexpr f32 PlayerBulletSize = 5.0f;
    static constexpr f32 BossBulletSize = 10.0f;

    MeshInstStateID idInstBulletMesh = 0;
    RenderInstEntityID idInstBullets = 0;
    DArray<InstRectColorData> BulletInstDrawData;
    int NumBulletsPlayer = 0;
    int NumBulletsBoss = 0;
    DArray<PerBulletData> ActiveBullets;

    static bool DoesCollide(PerBulletData& Bullet, AABB* BoundingBox);
    static bool IsOffscreen(PerBulletData& Bullet);
    void NewBullet(UBGameImpl* Game, BulletType Type, v2f Pos, v2f Vel);
    void Init(UBGameImpl* Game);
    void Term(UBGameImpl* Game);
    void Update(UBGameImpl* Game);
};

struct DebugVisualizer
{
    RenderInstEntityID idInstBBs = 0;
    DArray<InstRectColorData> BoundingBoxDraws;

    void Init(UBGameImpl* Game);
    void Term(UBGameImpl* Game);
    void Update(UBGameImpl* Game);
};

/*
struct PerParticleData
{
};

struct ParticleEmitter
{
    MeshInstStateID idInstParticleMesh = 0;
    RenderInstEntityID idInstParticles = 0;
    DArray<InstRectColorData> ParticleInstDrawData;
    DArray<PerParticleData> ActiveParticles;

    void Init(UBGameImpl* Game);
    void Term(UBGameImpl* Game);
    void Update(UBGameImpl* Game);
};
*/

struct Background
{
    enum struct ColorMode { Black, RandomDim, EmulateSound };
    ColorMode Mode = ColorMode::EmulateSound;

    MeshStateID idBackgroundMesh;
    RenderEntityID idBackground;
    // ColorMode::RandomDim state:
    v4f CurrColor;
    v4f NextColor;
    f32 LastSwitchTime;
    f32 StepDurationSeconds;
    // ColorMode::EmulateSound state:
    f32 BeatsPerMinute = 120.0f;

    // 3D background:
    MeshStateID idGridMesh = 0;
    RenderEntityID idGrid = 0;
    size_t NumCellsX = 0;
    size_t NumCellsY = 0;
    size_t NumVerts = 0;
    VxMin* GridMeshVerts = nullptr;

    void Init(UBGameImpl* Game);
    void Term(UBGameImpl* Game);
    void Update(UBGameImpl* Game);
};

struct UBGameImpl
{
    GfxSystem Gfx;

    Background BG;
    DebugVisualizer DbgVis;
    BulletManager BulletMgr;
    PlayerShip Player;
    BossShip Boss;

    bool Init();
    void Update();
    void Draw();
    bool Term();
};

#endif // GAME_IMPL_H

