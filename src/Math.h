#ifndef MATH_H
#define MATH_H

float LengthSq(v2f V);
float LengthSq(v3f V);
float Length(v2f V);
float Length(v3f V);
v2f Norm(v2f V);
v3f Norm(v3f V);

f32 Lerp(f32 A, f32 B, f32 t);
v2f Lerp(const v2f& A, const v2f& B, f32 t);
v3f Lerp(const v3f& A, const v3f& B, f32 t);
v4f Lerp(const v4f& A, const v4f& B, f32 t);

m2f Mult(const m2f& A, const m2f& B);
m3f Mult(const m3f& A, const m3f& B);
m4f Mult(const m4f& A, const m4f& B);
m2f operator*(const m2f& A, const m2f& B);
m3f operator*(const m3f& A, const m3f& B);
m4f operator*(const m4f& A, const m4f& B);

#include <math.h>

#endif // MATH_H

