#ifndef UBG_GFXDX11_H
#define UBG_GFXDX11_H

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>

struct UBG_Gfx_DX11
{
    static ID3D11Device* Device;
    static ID3D11DeviceContext* Context;
    static D3D_FEATURE_LEVEL FeatureLevel;
    static IDXGISwapChain1* SwapChain;
    static ID3D11Texture2D* BackBuffer;
    static ID3D11RenderTargetView* RenderTargetView;
    static ID3D11RasterizerState* RasterState;
    static ID3D11Texture2D* DepthStencil;
    static ID3D11DepthStencilView* DepthStencilView;

    static void DrawBegin();
    static void DrawEnd();
    static void Draw();
    static bool Init();
    static bool Term();
};

using UBG_GfxT = UBG_Gfx_DX11;

#endif // UBG_GFXDX11_H

