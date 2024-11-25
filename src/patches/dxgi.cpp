#define CINTERFACE
#define D3D11_NO_HELPERS
#define INITGUID

#include "d3d11.h"
#include "d3d12.h"
#include "dxgi1_2.h"
#include "dxgi1_3.h"
#include "dxgi1_4.h"
#include "dxgi1_5.h"
#include "dxgi1_6.h"
#include "helpers.h"
#include <MinHook.h>

#include "bnusio.h"
#include "patches.h"
#include <intrin.h>

/*
 * Reference: https://github.com/teknogods/OpenParrot/blob/master/OpenParrot/src/Functions/WindowedDxgi.cpp
 */

namespace patches::Dxgi {

// Local variables
static bool FpsLimiterEnable = false;

// Prototypes
static HRESULT (STDMETHODCALLTYPE *g_oldCreateSwapChain) (IDXGIFactory *This, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc,
                                                          IDXGISwapChain **ppSwapChain);
static HRESULT (STDMETHODCALLTYPE *g_oldCreateSwapChainForHwnd) (IDXGIFactory2 *This, IUnknown *pDevice, HWND hWnd,
                                                                 const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                                                 const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc,
                                                                 IDXGIOutput *pRestrictToOutput, IDXGISwapChain1 **ppSwapChain);
static HRESULT (STDMETHODCALLTYPE *g_oldPresentWrap) (IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);
static HRESULT (STDMETHODCALLTYPE *g_oldPresent1Wrap) (IDXGISwapChain1 *pSwapChain, UINT SyncInterval, UINT Flags);
static HRESULT (STDMETHODCALLTYPE *g_oldCreateSwapChain2) (IDXGIFactory2 *This, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc,
                                                           IDXGISwapChain **ppSwapChain);
static HRESULT (WINAPI *g_origCreateDXGIFactory2) (UINT Flags, REFIID riid, void **ppFactory);
static HRESULT (WINAPI *g_origCreateDXGIFactory) (REFIID, void **);
static HRESULT (WINAPI *g_origD3D11CreateDeviceAndSwapChain) (IDXGIAdapter *pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
                                                              const D3D_FEATURE_LEVEL *pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
                                                              /*const*/ DXGI_SWAP_CHAIN_DESC *pSwapChainDesc, IDXGISwapChain **ppSwapChain,
                                                              ID3D11Device **ppDevice, D3D_FEATURE_LEVEL *pFeatureLevel,
                                                              ID3D11DeviceContext **ppImmediateContext);

static HRESULT STDMETHODCALLTYPE CreateSwapChainWrap (IDXGIFactory *This, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc,
                                                      IDXGISwapChain **ppSwapChain);
static HRESULT STDMETHODCALLTYPE CreateSwapChainForHwndWrap (IDXGIFactory2 *This, IUnknown *pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                                             const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc, IDXGIOutput *pRestrictToOutput,
                                                             IDXGISwapChain1 **ppSwapChain);
static HRESULT STDMETHODCALLTYPE PresentWrap (IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);
static HRESULT STDMETHODCALLTYPE Present1Wrap (IDXGISwapChain1 *pSwapChain, UINT SyncInterval, UINT Flags);
static HRESULT STDMETHODCALLTYPE CreateSwapChain2Wrap (IDXGIFactory2 *This, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc,
                                                       IDXGISwapChain **ppSwapChain);
static HRESULT WINAPI CreateDXGIFactory2Wrap (UINT Flags, REFIID riid, void **ppFactory);
static HRESULT WINAPI CreateDXGIFactoryWrap (REFIID riid, _COM_Outptr_ void **ppFactory);
static HRESULT WINAPI D3D11CreateDeviceAndSwapChainWrap (IDXGIAdapter *pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
                                                         const D3D_FEATURE_LEVEL *pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
                                                         /*const*/ DXGI_SWAP_CHAIN_DESC *pSwapChainDesc, IDXGISwapChain **ppSwapChain,
                                                         ID3D11Device **ppDevice, D3D_FEATURE_LEVEL *pFeatureLevel,
                                                         ID3D11DeviceContext **ppImmediateContext);

// Functions
template <typename T>
inline T
HookVtableFunction (T *functionPtr, T target) {
    if (*functionPtr == target) return nullptr;

    auto old = *functionPtr;
    WRITE_MEMORY (functionPtr, T, target);

    return old;
}

static HRESULT STDMETHODCALLTYPE
CreateSwapChainWrap (IDXGIFactory *This, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain) {
    HRESULT hr = g_oldCreateSwapChain (This, pDevice, pDesc, ppSwapChain);

    if (*ppSwapChain) {
        if (FpsLimiterEnable) {
            auto old2        = HookVtableFunction (&(*ppSwapChain)->lpVtbl->Present, PresentWrap);
            g_oldPresentWrap = (old2) ? old2 : g_oldPresentWrap;
        }
    }

    return hr;
}

static HRESULT STDMETHODCALLTYPE
CreateSwapChainForHwndWrap (IDXGIFactory2 *This, IUnknown *pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                            const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc, IDXGIOutput *pRestrictToOutput, IDXGISwapChain1 **ppSwapChain) {
    HRESULT hr = g_oldCreateSwapChainForHwnd (This, pDevice, hWnd, pDesc, NULL, pRestrictToOutput, ppSwapChain);

    if (*ppSwapChain) {
        if (FpsLimiterEnable) {
            auto old2         = HookVtableFunction (&(*ppSwapChain)->lpVtbl->Present, Present1Wrap);
            g_oldPresent1Wrap = (old2) ? old2 : g_oldPresent1Wrap;
        }
    }

    return hr;
}

static HRESULT STDMETHODCALLTYPE
PresentWrap (IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags) {
    if (FpsLimiterEnable) patches::FpsLimiter::Update ();

    bnusio::Update ();

    return g_oldPresentWrap (pSwapChain, SyncInterval, Flags);
}

static HRESULT STDMETHODCALLTYPE
Present1Wrap (IDXGISwapChain1 *pSwapChain, UINT SyncInterval, UINT Flags) {
    if (FpsLimiterEnable) patches::FpsLimiter::Update ();

    bnusio::Update ();

    return g_oldPresent1Wrap (pSwapChain, SyncInterval, Flags);
}

static HRESULT STDMETHODCALLTYPE
CreateSwapChain2Wrap (IDXGIFactory2 *This, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain) {
    HRESULT hr = g_oldCreateSwapChain2 (This, pDevice, pDesc, ppSwapChain);

    if (*ppSwapChain) {
        if (FpsLimiterEnable) {
            auto old2        = HookVtableFunction (&(*ppSwapChain)->lpVtbl->Present, PresentWrap);
            g_oldPresentWrap = (old2) ? old2 : g_oldPresentWrap;
        }
    }

    return hr;
}

static HRESULT WINAPI
CreateDXGIFactory2Wrap (UINT Flags, REFIID riid, void **ppFactory) {
    HRESULT hr = g_origCreateDXGIFactory2 (Flags, riid, ppFactory);

    if (SUCCEEDED (hr)) {
        IDXGIFactory2 *factory = (IDXGIFactory2 *)*ppFactory;

        auto old              = HookVtableFunction (&factory->lpVtbl->CreateSwapChain, CreateSwapChain2Wrap);
        g_oldCreateSwapChain2 = (old) ? old : g_oldCreateSwapChain2;
    }

    return hr;
}

static HRESULT WINAPI
CreateDXGIFactoryWrap (REFIID riid, _COM_Outptr_ void **ppFactory) {
    HRESULT hr = g_origCreateDXGIFactory (riid, ppFactory);

    if (SUCCEEDED (hr)) {
        int factoryType = 0;

        if (IsEqualIID (riid, IID_IDXGIFactory1)) factoryType = 1;
        else if (IsEqualIID (riid, IID_IDXGIFactory2)) factoryType = 2;
        else if (IsEqualIID (riid, IID_IDXGIFactory3)) factoryType = 3;
        else if (IsEqualIID (riid, IID_IDXGIFactory4)) factoryType = 4;
        else if (IsEqualIID (riid, IID_IDXGIFactory5)) factoryType = 5;
        else if (IsEqualIID (riid, IID_IDXGIFactory6)) factoryType = 6;
        else if (IsEqualIID (riid, IID_IDXGIFactory7)) factoryType = 7;

        if (factoryType >= 0) {
            IDXGIFactory *factory = (IDXGIFactory *)*ppFactory;

            auto old             = HookVtableFunction (&factory->lpVtbl->CreateSwapChain, CreateSwapChainWrap);
            g_oldCreateSwapChain = (old) ? old : g_oldCreateSwapChain;
        }

        if (factoryType >= 2) {
            IDXGIFactory2 *factory = (IDXGIFactory2 *)*ppFactory;

            auto old                    = HookVtableFunction (&factory->lpVtbl->CreateSwapChainForHwnd, CreateSwapChainForHwndWrap);
            g_oldCreateSwapChainForHwnd = (old) ? old : g_oldCreateSwapChainForHwnd;
        }
    }

    return hr;
}

static HRESULT WINAPI
D3D11CreateDeviceAndSwapChainWrap (IDXGIAdapter *pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
                                   const D3D_FEATURE_LEVEL *pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
                                   /*const*/ DXGI_SWAP_CHAIN_DESC *pSwapChainDesc, IDXGISwapChain **ppSwapChain, ID3D11Device **ppDevice,
                                   D3D_FEATURE_LEVEL *pFeatureLevel, ID3D11DeviceContext **ppImmediateContext) {
    HRESULT hr = g_origD3D11CreateDeviceAndSwapChain (pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion,
                                                      pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);

    if (ppSwapChain) {
        if (FpsLimiterEnable) {
            auto old2        = HookVtableFunction (&(*ppSwapChain)->lpVtbl->Present, PresentWrap);
            g_oldPresentWrap = (old2) ? old2 : g_oldPresentWrap;
        }
    }

    return hr;
}

void
Init () {
    LogMessage (LOG_LEVEL_DEBUG, "Init Dxgi patches");
    i32 fpsLimit = 120;

    auto configPath = std::filesystem::current_path () / "config.toml";
    std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);
    if (config_ptr) {
        auto graphics = openConfigSection (config_ptr.get (), "graphics");
        if (graphics) fpsLimit = readConfigInt (graphics, "fpslimit", fpsLimit);
    }

    FpsLimiterEnable = fpsLimit > 0;
    patches::FpsLimiter::Init ((float)fpsLimit);

    MH_Initialize ();
    MH_CreateHookApi (L"dxgi.dll", "CreateDXGIFactory", (LPVOID)CreateDXGIFactoryWrap, (void **)&g_origCreateDXGIFactory);
    MH_CreateHookApi (L"dxgi.dll", "CreateDXGIFactory2", (LPVOID)CreateDXGIFactory2Wrap, (void **)&g_origCreateDXGIFactory2);
    MH_CreateHookApi (L"d3d11.dll", "D3D11CreateDeviceAndSwapChain", (LPVOID)D3D11CreateDeviceAndSwapChainWrap,
                      (void **)&g_origD3D11CreateDeviceAndSwapChain);
    MH_EnableHook (MH_ALL_HOOKS);
}

} // namespace patches::Dxgi