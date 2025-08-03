#ifndef UBG_H
#define UBG_H

// C/C++ std lib headers:
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

struct GlobalState
{
    static bool bRunning;
    static int Width;
    static int Height;
};

extern void Outf(const char* Fmt, ...);
#define DEBUG_BREAKPOINT() DebugBreak()
#define UNUSED_VAR(Var) (void)Var
#define ASSERT(Expr) if (!(Expr)) { Outf("[assert] FAILED\n\tExp: %s\n\tFile: %s\tLine: %d\n\tFunc: %s", #Expr, __FILE__, __LINE__, __FUNCSIG__); DEBUG_BREAKPOINT(); }
#define ARRAY_SIZE(Array) (sizeof((Array)) / sizeof((Array)[0]))

// UBG Platform impl
#include "UBG_Platform.h"
// UBG Graphics backend impl
#include "UBG_Gfx.h"
// Common project headers
#include "Clock.h"
#include "Math.h"

#endif // UBG_H

