#include "UBG.h" // Includes Collision.h

bool Check(v2f Point, AABB* A)
{
    if (!A) { return false; }
    return A->Min.X < Point.X && Point.X < A->Max.X &&
        A->Min.Y < Point.Y && Point.Y < A->Max.Y;
}

bool Check(v2f Point, SphereBB* A)
{
    if (!A) { return false; }
    return Dist(Point, A->Pos) < A->Size;
}

bool Collision::Check(AABB* A, AABB* B)
{
    if (!A || !B) { return false; }
    return (A->Min.X < B->Max.X && B->Min.X < A->Max.X) &&
        (A->Min.Y < B->Max.Y && B->Min.Y < A->Max.Y);
}

bool Collision::Check(SphereBB* A, SphereBB* B)
{
    if (!A || !B) { return false; }
    return Dist(A->Pos, B->Pos) < (A->Size + B->Size);
}

bool Collision::Check(AABB* A, SphereBB* B)
{
    v2f ClosestPoint =
    {
        Max(A->Min.X, Min(B->Pos.X, A->Max.X)),
        Max(A->Min.Y, Min(B->Pos.Y, A->Max.Y)),
    };
    return Dist(ClosestPoint, B->Pos) < B->Size;
}

