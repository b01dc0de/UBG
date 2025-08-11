#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// Fixed-width numeric types:
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using f32 = float;
using f64 = double;

// Vector / matrix types:
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

    static m2f Identity();
};

struct m3f
{
    v3f V0;
    v3f V1;
    v3f V2;

    static m3f Identity();
};

struct m4f
{
    v4f V0;
    v4f V1;
    v4f V2;
    v4f V3;

    static m4f Identity();
    static m4f Scale(f32 X, f32 Y, f32 Z);
    static m4f Trans(f32 X, f32 Y, f32 Z);
};

#endif // TYPES_H

