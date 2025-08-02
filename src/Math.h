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

float lerp(float A, float B, float t);
void lerp(const v4f& A, const v4f& B, float t, v4f& C);

#endif // MATH_H

