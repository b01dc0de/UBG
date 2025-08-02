#ifndef UBG_H
#define UBG_H

// C/C++ std lib headers
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct GlobalState
{
    static bool bRunning;
};

extern void Outf(const char* Fmt, ...);
#define DEBUG_BREAKPOINT() DebugBreak()
#define UNUSED_VAR(Var) (void)Var
#define ASSERT(Expr) if (!(Expr)) { Outf("[assert] FAILED\n\tExp: %s\n\tFile: %s\tLine: %d\n\tFunc: %s", #Expr, __FILE__, __LINE__, __FUNCSIG__); DEBUG_BREAKPOINT(); }
#define ARRAY_SIZE(Array) (sizeof((Array)) / sizeof((Array)[0]))

// UBG Platform
#include "UBG_Platform.h"

#endif // UBG_H

