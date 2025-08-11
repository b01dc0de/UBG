#ifndef MATH_H
#define MATH_H

struct v2f
{
    f32 X;
    f32 Y;
};

struct v3f
{
    f32 X;
    f32 Y;
    f32 Z;
};

struct v4f
{
    f32 X;
    f32 Y;
    f32 Z;
    f32 W;
};

struct m2f
{
    v2f V0;
    v2f V1;

    static m2f Identity()
    {
        m2f Result = 
        {
            { 1.0f, 0.0f },
            { 0.0f, 1.0f }
        };
        return Result;
    }
};

struct m3f
{
    v3f V0;
    v3f V1;
    v3f V2;

    static m3f Identity()
    {
        m3f Result = 
        {
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f }
        };
        return Result;
    }
};

struct m4f
{
    v4f V0;
    v4f V1;
    v4f V2;
    v4f V3;

    static m4f Identity()
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

    static m4f Scale(f32 X, f32 Y, f32 Z)
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

    static m4f Trans(f32 X, f32 Y, f32 Z)
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
};

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

