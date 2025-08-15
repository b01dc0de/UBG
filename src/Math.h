#ifndef MATH_H
#define MATH_H

static constexpr f32 fPI = 3.14159265359f;
static constexpr f32 fTAU = fPI * 2.0f;

bool fIsZero(f32 A);

float Clamp(f32 Min, float Max, float C);

v2f Add(v2f A, v2f B);
v3f Add(v3f A, v3f B);
v2f Subtract(v2f A, v2f B);
v3f Subtract(v3f A, v3f B);

float LengthSq(v2f V);
float LengthSq(v3f V);
float Length(v2f V);
float Length(v3f V);
v2f Norm(v2f V);
v3f Norm(v3f V);

float Dot(v2f A, v2f B);
float Dot(v3f A, v3f B);
float Dot(v4f A, v4f B);
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

