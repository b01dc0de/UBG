#ifndef MATH_H
#define MATH_H

f32 lerp(f32 A, f32 B, f32 t);
v2f lerp(const v2f& A, const v2f& B, f32 t);
v3f lerp(const v3f& A, const v3f& B, f32 t);
v4f lerp(const v4f& A, const v4f& B, f32 t);

m2f Mult(const m2f& A, const m2f& B);
m3f Mult(const m3f& A, const m3f& B);
m4f Mult(const m4f& A, const m4f& B);
m2f operator*(const m2f& A, const m2f& B);
m3f operator*(const m3f& A, const m3f& B);
m4f operator*(const m4f& A, const m4f& B);

#endif // MATH_H

