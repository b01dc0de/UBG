#ifndef COLLISION_H
#define COLLISION_H

struct AABB
{
    v2f Min;
    v2f Max;
};

struct SphereBB
{
    v2f Pos;
    f32 Size;
};

struct Collision
{
    static bool Check(v2f Point, AABB* A);
    static bool Check(v2f Point, SphereBB* A);

    static bool Check(AABB* A, AABB* B);
    static bool Check(SphereBB* A, SphereBB* B);
    static bool Check(AABB* A, SphereBB* B);
};

#endif // COLLISION_H

