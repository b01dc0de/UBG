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

m2f Mult(const m2f& A, const m2f& B)
{
    m2f Result;

    Result.V0.X = A.V0.X * B.V0.X + A.V0.Y * B.V1.X;
    Result.V0.Y = A.V0.X * B.V0.Y + A.V0.Y * B.V1.Y;

    Result.V1.X = A.V1.X * B.V0.X + A.V1.Y * B.V1.X;
    Result.V1.Y = A.V1.X * B.V0.Y + A.V1.Y * B.V1.Y;

    return Result;
}

m3f Mult(const m3f& A, const m3f& B)
{
    m3f Result;

    Result.V0.X = A.V0.X * B.V0.X + A.V0.Y * B.V1.X + A.V0.Z * B.V2.X;
    Result.V0.Y = A.V0.X * B.V0.Y + A.V0.Y * B.V1.Y + A.V0.Z * B.V2.Y;
    Result.V0.Z = A.V0.X * B.V0.Z + A.V0.Y * B.V1.Z + A.V0.Z * B.V2.Z;

    Result.V1.X = A.V1.X * B.V0.X + A.V1.Y * B.V1.X + A.V1.Z * B.V2.X;
    Result.V1.Y = A.V1.X * B.V0.Y + A.V1.Y * B.V1.Y + A.V1.Z * B.V2.Y;
    Result.V1.Z = A.V1.X * B.V0.Z + A.V1.Y * B.V1.Z + A.V1.Z * B.V2.Z;

    Result.V2.X = A.V2.X * B.V0.X + A.V2.Y * B.V1.X + A.V2.Z * B.V2.X;
    Result.V2.Y = A.V2.X * B.V0.Y + A.V2.Y * B.V1.Y + A.V2.Z * B.V2.Y;
    Result.V2.Z = A.V2.X * B.V0.Z + A.V2.Y * B.V1.Z + A.V2.Z * B.V2.Z;

    return Result;
}

m4f Mult(const m4f& A, const m4f& B)
{
    m4f Result;

    Result.V0.X = A.V0.X * B.V0.X + A.V0.Y * B.V1.X + A.V0.Z * B.V2.X + A.V0.W * B.V3.X;
    Result.V0.Y = A.V0.X * B.V0.Y + A.V0.Y * B.V1.Y + A.V0.Z * B.V2.Y + A.V0.W * B.V3.Y;
    Result.V0.Z = A.V0.X * B.V0.Z + A.V0.Y * B.V1.Z + A.V0.Z * B.V2.Z + A.V0.W * B.V3.Z;
    Result.V0.W = A.V0.X * B.V0.W + A.V0.Y * B.V1.W + A.V0.Z * B.V2.W + A.V0.W * B.V3.W;

    Result.V1.X = A.V1.X * B.V0.X + A.V1.Y * B.V1.X + A.V1.Z * B.V2.X + A.V1.W * B.V3.X;
    Result.V1.Y = A.V1.X * B.V0.Y + A.V1.Y * B.V1.Y + A.V1.Z * B.V2.Y + A.V1.W * B.V3.Y;
    Result.V1.Z = A.V1.X * B.V0.Z + A.V1.Y * B.V1.Z + A.V1.Z * B.V2.Z + A.V1.W * B.V3.Z;
    Result.V1.W = A.V1.X * B.V0.W + A.V1.Y * B.V1.W + A.V1.Z * B.V2.W + A.V1.W * B.V3.W;

    Result.V2.X = A.V2.X * B.V0.X + A.V2.Y * B.V1.X + A.V2.Z * B.V2.X + A.V2.W * B.V3.X;
    Result.V2.Y = A.V2.X * B.V0.Y + A.V2.Y * B.V1.Y + A.V2.Z * B.V2.Y + A.V2.W * B.V3.Y;
    Result.V2.Z = A.V2.X * B.V0.Z + A.V2.Y * B.V1.Z + A.V2.Z * B.V2.Z + A.V2.W * B.V3.Z;
    Result.V2.W = A.V2.X * B.V0.W + A.V2.Y * B.V1.W + A.V2.Z * B.V2.W + A.V2.W * B.V3.W;

    Result.V3.X = A.V3.X * B.V0.X + A.V3.Y * B.V1.X + A.V3.Z * B.V2.X + A.V3.W * B.V3.X;
    Result.V3.Y = A.V3.X * B.V0.Y + A.V3.Y * B.V1.Y + A.V3.Z * B.V2.Y + A.V3.W * B.V3.Y;
    Result.V3.Z = A.V3.X * B.V0.Z + A.V3.Y * B.V1.Z + A.V3.Z * B.V2.Z + A.V3.W * B.V3.Z;
    Result.V3.W = A.V3.X * B.V0.W + A.V3.Y * B.V1.W + A.V3.Z * B.V2.W + A.V3.W * B.V3.W;

    return Result;
}

m2f operator*(const m2f& A, const m2f& B)
{
    return Mult(A, B);
}

m3f operator*(const m3f& A, const m3f& B)
{
    return Mult(A, B);
}

m4f operator*(const m4f& A, const m4f& B)
{
    return Mult(A, B);
}

