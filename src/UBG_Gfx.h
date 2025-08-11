#ifndef UBG_GFX_H
#define UBG_GFX_H

#define UBG_GFX_IMPL_DX11() (1)

#include "UBG_GfxCommon.h"

#if UBG_GFX_IMPL_DX11()
    #include "UBG_GfxDX11.h"
#else // UBG_GFX_IMPL_
    static_assert(false, "[build] Undefined graphics backend!");
#endif //UBG_GFX_IMPL_

#endif // UBG_GFX_H

