#ifndef MATH_H
#define MATH_H

struct v2f
{
    float X;
    float Y;
};

struct v3f
{
    float X;
    float Y;
    float Z;
};

struct v4f
{
    float X;
    float Y;
    float Z;
    float W;
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
};

float lerp(float A, float B, float t);
void lerp(const v4f& A, const v4f& B, float t, v4f& C);

#endif // MATH_H

