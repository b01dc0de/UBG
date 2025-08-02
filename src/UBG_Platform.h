#ifndef UBG_PLATFORM_H
#define UBG_PLATFORM_H

#define UBG_PLATFORM_WIN32() (1)

#if UBG_PLATFORM_WIN32()
    #include "UBG_PlatformWin32.h"
#elif // UBG_PLATFORM_
    static_assert(0, "[build] Unsupported platform");
#endif // UBG_PLATFORM_

#endif // UBG_PLATFORM_H

