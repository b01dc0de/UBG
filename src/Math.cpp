#include "UBG.h" // Includes Math.h

m2f m2f::Identity()
{
    m2f Result =
    {
        { 1.0f, 0.0f },
        { 0.0f, 1.0f }
    };
    return Result;
}

m3f m3f::Identity()
{
    m3f Result =
    {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f }
    };
    return Result;
}

m4f m4f::Identity()
{
    m4f Result =
    {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    return Result;
}

m4f m4f::Scale(f32 X, f32 Y, f32 Z)
{
    m4f Result =
    {
        { X, 0.0f, 0.0f, 0.0f },
        { 0.0f, Y, 0.0f, 0.0f },
        { 0.0f, 0.0f, Z, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    return Result;
}

m4f m4f::Trans(f32 X, f32 Y, f32 Z)
{
    m4f Result =
    {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { X, Y, Z, 1.0f }
    };
    return Result;
}

f32 lerp(f32 A, f32 B, f32 t)
{
    return A + t * (B - A);
}

v2f lerp(const v2f& A, const v2f& B, f32 t)
{
    v2f Result = {
        lerp(A.X, B.X, t),
        lerp(A.Y, B.Y, t)
    };
    return Result;
}

v3f lerp(const v3f& A, const v3f& B, f32 t)
{
    v3f Result = {
        lerp(A.X, B.X, t),
        lerp(A.Y, B.Y, t),
        lerp(A.Z, B.Z, t)
    };
    return Result;
}

v4f lerp(const v4f& A, const v4f& B, f32 t)
{
    v4f Result = {
        lerp(A.X, B.X, t),
        lerp(A.Y, B.Y, t),
        lerp(A.Z, B.Z, t),
        lerp(A.W, B.W, t)
    };
    return Result;
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

