#include "UBG.h" // Includes Math.h

#include <float.h>

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

m4f m4f::RotX(f32 Angle)
{
    const f32 fCos = cosf(Angle);
    const f32 fSin = sinf(Angle);

    m4f Result = Identity();
    Result.V1 = { 0.0f,  fCos, -fSin, 0.0f };
    Result.V2 = { 0.0f,  fSin,  fCos, 0.0f };
    return Result;
}

m4f m4f::RotY(f32 Angle)
{
    const f32 fCos = cosf(Angle);
    const f32 fSin = sinf(Angle);

    m4f Result = Identity();
    Result.V0 = { fCos, 0.0f,  fSin, 0.0f };
    Result.V2 = { -fSin, 0.0f,  fCos, 0.0f };
    return Result;
}

m4f m4f::RotZ(f32 Angle)
{
    const f32 fCos = cosf(Angle);
    const f32 fSin = sinf(Angle);

    m4f Result = Identity();
    Result.V0 = { fCos,  -fSin, 0.0f, 0.0f };
    Result.V1 = { fSin,   fCos, 0.0f, 0.0f };
    return Result;
}

m4f m4f::RotAxis(v3f Axis, f32 Angle)
{
    v3f k = Norm(Axis);

    f32 C = cosf(Angle);
    f32 S = sinf(Angle);

    f32 c_inv = 1.0f - C;

    f32 k_xx = k.X * k.X;
    f32 k_yy = k.Y * k.Y;
    f32 k_zz = k.Z * k.Z;

    f32 k_xy = k.X * k.Y;
    f32 k_yz = k.Y * k.Z;
    f32 k_xz = k.X * k.Z;

    f32 s_x = S * k.X;
    f32 s_y = S * k.Y;
    f32 s_z = S * k.Z;

    m4f Result;
    Result.V0 = { C + k_xx * c_inv, +s_z + k_xy * c_inv, -s_y + k_xz * c_inv, 0.0f };
    Result.V1 = { -s_z + k_xy * c_inv, C + k_yy * c_inv, +s_x + k_yz * c_inv, 0.0f };
    Result.V2 = { +s_y + k_xz * c_inv, -s_x + k_yz * c_inv, C + k_zz * c_inv, 0.0f };
    Result.V3 = { 0.0f, 0.0f, 0.0f, 1.0f };
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

bool fIsZero(f32 A)
{
    constexpr f32 fEpsilon = FLT_EPSILON;
    return fabsf(A) < fEpsilon;
}

f32 Min(f32 A, f32 B)
{
    if (B < A) { return B; }
    return A;
}

f32 Max(f32 A, f32 B)
{
    if (B > A) { return B; }
    return A;
}

f32 Clamp(f32 Min, f32 Max, f32 C)
{
    if (C < Min) { return Min; }
    if (C > Max) { return Max; }
    return C;
}

v2f Add(v2f A, v2f B)
{
    v2f Result = { A.X + B.X, A.Y + B.Y };
    return Result;
}

v3f Add(v3f A, v3f B)
{
    v3f Result = { A.X + B.X, A.Y + B.Y, A.Z + B.Z };
    return Result;
}

v2f operator+(v2f A, v2f B)
{
    return Add(A, B);
}

v3f operator+(v3f A, v3f B)
{
    return Add(A, B);
}

v2f& operator+=(v2f& A, v2f B)
{
    A = Add(A, B);
    return A;
}

v3f& operator+=(v3f& A, v3f B)
{
    A = Add(A, B);
    return A;
}

v2f Subtract(v2f A, v2f B)
{
    v2f Result = { A.X - B.X, A.Y - B.Y };
    return Result;
}

v3f Subtract(v3f A, v3f B)
{
    v3f Result = { A.X - B.X, A.Y - B.Y, A.Z - B.Z };
    return Result;
}

v2f operator-(v2f A, v2f B)
{
    return Subtract(A, B);
}

v3f operator-(v3f A, v3f B)
{
    return Subtract(A, B);
}

v2f& operator-=(v2f& A, v2f B)
{
    A = Subtract(A, B);
    return A;
}

v3f& operator-=(v3f& A, v3f B)
{
    A = Subtract(A, B);
    return A;
}

v2f Mult(v2f A, f32 S)
{
    v2f Result = { A.X * S, A.Y * S };
    return Result;
}

v3f Mult(v3f A, f32 S)
{
    v3f Result = { A.X * S, A.Y * S, A.Z * S };
    return Result;
}

v2f operator*(v2f A, f32 S)
{
    return Mult(A, S);
}

v3f operator*(v3f A, f32 S)
{
    return Mult(A, S);
}

v2f& operator*=(v2f& A, f32 S)
{
    A = Mult(A, S);
    return A;
}

v3f& operator*=(v3f& A, f32 S)
{
    A = Mult(A, S);
    return A;
}

v2f Divide(v2f A, f32 S)
{
    v2f Result = { A.X / S, A.Y / S };
    return Result;
}

v3f Divide(v3f A, f32 S)
{
    v3f Result = { A.X / S, A.Y / S, A.Z / S };
    return Result;
}

v2f operator/(v2f A, f32 S)
{
    return Divide(A, S);
}

v3f operator/(v3f A, f32 S)
{
    return Divide(A, S);
}

v2f& operator/=(v2f& A, f32 S)
{
    A = Divide(A, S);
    return A;
}

v3f& operator/=(v3f& A, f32 S)
{
    A = Divide(A, S);
    return A;
}

f32 LengthSq(v2f V)
{
    f32 Result = V.X * V.X + V.Y * V.Y;
    return Result;
}

f32 LengthSq(v3f V)
{
    f32 Result = V.X * V.X + V.Y * V.Y + V.Z * V.Z;
    return Result;
}

f32 Length(v2f V)
{
    f32 Result = sqrtf(LengthSq(V));
    return Result;
}

f32 Length(v3f V)
{
    f32 Result = sqrtf(LengthSq(V));
    return Result;
}

v2f Norm(v2f V)
{
    f32 fLength = Length(V);
    v2f Result = { 0.0f, 0.0f };
    if (!fIsZero(fLength))
    {
        Result = { V.X / fLength, V.Y / fLength };
    }
    return Result;
}

v3f Norm(v3f V)
{
    f32 fLength = Length(V);
    v3f Result = { 0.0f, 0.0f, 0.0f };
    if (!fIsZero(fLength))
    {
        Result = { V.X / fLength, V.Y / fLength, V.Z / fLength };
    }
    return Result;
}

f32 Dist(v2f A, v2f B)
{
    v2f Diff = A - B;
    return Length(Diff);
}

f32 Dist(v3f A, v3f B)
{
    v3f Diff = A - B;
    return Length(Diff);
}

f32 Dot(v2f A, v2f B)
{
    f32 Result = A.X * B.X + A.Y * B.Y;
    return Result;
}

f32 Dot(v3f A, v3f B)
{
    f32 Result = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    return Result;
}

f32 Dot(v4f A, v4f B)
{
    f32 Result = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Result;
}

v3f Cross(v3f A, v3f B)
{
    v3f Result = {
        A.Y * B.Z - A.Z * B.Y,
        A.Z * B.X - A.X * B.Z,
        A.X * B.Y - A.Y * B.X,
    };
    return Result;
}

f32 Lerp(f32 A, f32 B, f32 t)
{
    return A + t * (B - A);
}

v2f Lerp(v2f A, v2f B, f32 t)
{
    v2f Result = {
        Lerp(A.X, B.X, t),
        Lerp(A.Y, B.Y, t)
    };
    return Result;
}

v3f Lerp(v3f A, v3f B, f32 t)
{
    v3f Result = {
        Lerp(A.X, B.X, t),
        Lerp(A.Y, B.Y, t),
        Lerp(A.Z, B.Z, t)
    };
    return Result;
}

v4f Lerp(v4f A, v4f B, f32 t)
{
    v4f Result = {
        Lerp(A.X, B.X, t),
        Lerp(A.Y, B.Y, t),
        Lerp(A.Z, B.Z, t),
        Lerp(A.W, B.W, t)
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

