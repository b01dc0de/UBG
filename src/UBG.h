#ifndef UBG_H
#define UBG_H

// TODO: Define _precisely_ exactly what structs/functions Platforms and GraphicsBackends need to implement/override
extern void Outf(const char* Fmt, ...);
extern void DebugBreakpoint();
#define DEBUG_BREAKPOINT() DebugBreakpoint()
#define UNUSED_VAR(Var) (void)Var
#define ASSERT(Expr) if (!(Expr)) { Outf("[assert] FAILED\n\tExp: %s\n\tFile: %s\tLine: %d\n\tFunc: %s", #Expr, __FILE__, __LINE__, __FUNCSIG__); DEBUG_BREAKPOINT(); }
#define ARRAY_SIZE(Array) (sizeof((Array)) / sizeof((Array)[0]))

// Engine definition
struct UBG_Engine
{
    bool bRunning;
    int Width;
    int Height;
    struct UBGame* Instance;

    void GameLoop();
    bool Init();
    bool Term();
};
extern UBG_Engine* GlobalEngine;

// Common types:
#include "UBG_Types.h"
// UBG Platform impl:
#include "UBG_Platform.h"
// UBG Graphics backend impl:
#include "UBG_Gfx.h"
// Common project headers:
#include "Array.h"
#include "Clock.h"
#include "Input.h"
#include "Math.h"
#include "Memory.h"
#include "Utils.h"
// The Game:
#include "UBGame.h"

#endif // UBG_H

