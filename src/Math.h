#ifndef MATH_H
#define MATH_H

static constexpr f32 fPI = 3.14159265359f;
static constexpr f32 fTAU = fPI * 2.0f;
static constexpr f32 fSqrt2 = 1.414213562373095f;
static constexpr f32 fInvSqrt2 = 1.0f / fSqrt2;

bool fIsZero(f32 A);

f32 Square(f32 A);

template <typename T>
T Min(T A, T B)
{
    if (B < A) { return B; }
    return A;
}

template <typename T>
T Max(T A, T B)
{
    if (B > A) { return B; }
    return A;
}

template <typename T>
T Clamp(T Min, T Max, T C)
{
    if (C < Min) { return Min; }
    if (C > Max) { return Max; }
    return C;
}

/*
f32 Min(f32 A, f32 B);
f32 Max(f32 A, f32 B);
f32 Clamp(f32 Min, f32 Max, f32 C);
*/

inline f32 Abs(f32 A)
{
    if (A < 0.0f) { return -A; }
    return A;
}

inline f64 Abs(f64 A)
{
    if (A < 0.0f) { return -A; }
    return A;
}

bool Equals(v2i A, v2i B);
bool NotEquals(v2i A, v2i B);
bool operator==(v2i A, v2i B);
bool operator!=(v2i A, v2i B);

v2i Negate(v2i A);
v2f Negate(v2f A);
v3f Negate(v3f A);
v2i operator-(v2i A);
v2f operator-(v2f A);
v3f operator-(v3f A);

v2i Add(v2i A, v2i B);
v2f Add(v2f A, v2f B);
v3f Add(v3f A, v3f B);
v2i operator+(v2i A, v2i B);
v2f operator+(v2f A, v2f B);
v3f operator+(v3f A, v3f B);
v2i& operator+=(v2i& A, v2i B);
v2f& operator+=(v2f& A, v2f B);
v3f& operator+=(v3f& A, v3f B);

v2i Subtract(v2i A, v2i B);
v2f Subtract(v2f A, v2f B);
v3f Subtract(v3f A, v3f B);
v2i operator-(v2i A, v2i B);
v2f operator-(v2f A, v2f B);
v3f operator-(v3f A, v3f B);
v2i& operator-=(v2i& A, v2i B);
v2f& operator-=(v2f& A, v2f B);
v3f& operator-=(v3f& A, v3f B);

v2i Mult(v2i A, s32 S);
v2f Mult(v2f A, f32 S);
v3f Mult(v3f A, f32 S);
v2i operator*(v2i A, s32 S);
v2f operator*(v2f A, f32 S);
v3f operator*(v3f A, f32 S);
v2i& operator*=(v2i& A, s32 S);
v2f& operator*=(v2f& A, f32 S);
v3f& operator*=(v3f& A, f32 S);

v2i Divide(v2i A, s32 S);
v2f Divide(v2f A, f32 S);
v3f Divide(v3f A, f32 S);
v2i operator/(v2i A, s32 S);
v2f operator/(v2f A, f32 S);
v3f operator/(v3f A, f32 S);
v2i& operator/=(v2i& A, s32 S);
v2f& operator/=(v2f& A, f32 S);
v3f& operator/=(v3f& A, f32 S);

f32 LengthSq(v2i V);
f32 LengthSq(v2f V);
f32 LengthSq(v3f V);
f32 Length(v2i V);
f32 Length(v2f V);
f32 Length(v3f V);
v2f Norm(v2f V);
v3f Norm(v3f V);

f32 Dist(v2i A, v2i B);
f32 Dist(v2f A, v2f B);
f32 Dist(v3f A, v3f B);

f32 Dot(v2f A, v2f B);
f32 Dot(v3f A, v3f B);
f32 Dot(v4f A, v4f B);
v3f Cross(v3f A, v3f B);

f32 Lerp(f32 A, f32 B, f32 t);
v2f Lerp(v2f A, v2f B, f32 t);
v3f Lerp(v3f A, v3f B, f32 t);
v4f Lerp(v4f A, v4f B, f32 t);

m2f Mult(const m2f& A, const m2f& B);
m3f Mult(const m3f& A, const m3f& B);
m4f Mult(const m4f& A, const m4f& B);
m2f operator*(const m2f& A, const m2f& B);
m3f operator*(const m3f& A, const m3f& B);
m4f operator*(const m4f& A, const m4f& B);

#include <math.h>

#endif // MATH_H

