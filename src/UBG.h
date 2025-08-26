#ifndef UBG_H
#define UBG_H

// TODO: Define _precisely_ exactly what structs/functions Platforms and GraphicsBackends need to implement/override
extern void Outf(const char* Fmt, ...);
extern void DebugBreakpoint();
#define DEBUG_BREAKPOINT() DebugBreakpoint()
#define UNUSED_VAR(Var) (void)Var
#define ASSERT(Expr) if (!(Expr)) { Outf("[assert] FAILED\n\tExp: %s\n\tFile: %s\tLine: %d\n\tFunc: %s", #Expr, __FILE__, __LINE__, __FUNCSIG__); DEBUG_BREAKPOINT(); }
#define ARRAY_SIZE(Array) (sizeof((Array)) / sizeof((Array)[0]))

// Common types:
#include "UBG_Types.h"
// UBG Platform impl:
#include "UBG_Platform.h"
// UBG Graphics backend impl:
#include "UBG_Gfx.h"

// Engine definition
struct UBG_Engine
{
    bool bRunning;
    s32 Width;
    s32 Height;
    u64 FrameNo;
    UBG_PlatformT* PlatformState;
    UBG_GfxT* GfxState;
    struct UBGame* Instance;
    struct ClockT* Clock;
    struct InputT* Input;

    void GameLoop();
    void Draw();
    bool Init();
    bool Term();
};
extern UBG_Engine* GlobalEngine;

// Common project headers:
#include "Array.h"
#include "Clock.h"
#include "Collision.h"
#include "Input.h"
#include "Math.h"
#include "Memory.h"
#include "Utils.h"
// The Game:
#include "UBGame.h"

#endif // UBG_H

