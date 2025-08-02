#ifndef UBG_PLATFORM_H
#define UBG_PLATFORM_H

#define UBG_PLATFORM_WIN32() (1)

#if UBG_PLATFORM_WIN32()
    #include "UBG_PlatformWin32.h"
#else // UBG_PLATFORM_
    static_assert(false, "[build] Undefined platform!");
#endif // UBG_PLATFORM_

#endif // UBG_PLATFORM_H
