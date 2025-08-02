#include "UBG.h" // UBG.h includes Math.h

float lerp(float A, float B, float t)
{
    return A + t * (B - A);
}

void lerp(const v4f& A, const v4f& B, float t, v4f& C)
{
    C.X = lerp(A.X, B.X, t);
    C.Y = lerp(A.Y, B.Y, t);
    C.Z = lerp(A.Z, B.Z, t);
    C.W = lerp(A.W, B.W, t);
}

