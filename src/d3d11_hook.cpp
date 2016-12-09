

#include <cstdint>
#include <d3d11.h>
#include <jc3_hooking/hooking.h>
#include <ksignals.h>

#include "d3d11_hook.h"

static D3D11HookEvents events;

static uintptr_t g_originalD3D11CreateDevice = 0;


// http://stackoverflow.com/questions/1888863/how-to-get-main-window-handle-from-process-id
HWND FindTopWindow(DWORD pid)
{
    std::pair<HWND, DWORD> params = { 0, pid };

    // Enumerate the windows using a lambda to process each window
    BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
    {
        auto pParams = (std::pair<HWND, DWORD>*)(lParam);

        DWORD processId;
        if (GetWindowThreadProcessId(hwnd, &processId) && processId == pParams->second)
        {
            // Stop enumerating
            SetLastError(-1);
            pParams->first = hwnd;
            return FALSE;
        }

        // Continue enumerating
        return TRUE;
    }, (LPARAM)&params);

    if (!bResult && GetLastError() == -1 && params.first)
    {
        return params.first;
    }

    return 0;
}

void HookJC3Present() {
#pragma pack(push, 1)
	struct AvalancheDXGIDevice
	{
		int32_t field_0;
		uint8_t gap4[28];
		IDXGISwapChain *swapChain;
		ID3D11Device *device;
		uint8_t gap30[372];
		int32_t syncInterval;
	};
#pragma pack(pop)

	//util::hooking::nop(0x143807DD5, 5);

	static util::hooking::inject_call<int64_t, AvalancheDXGIDevice*> injectPresent(0x1432E0071);
	injectPresent.inject([](AvalancheDXGIDevice *a1) -> int64_t {

        static std::once_flag flag;
        std::call_once(flag, [a1]() {
            HWND focusWindow = FindTopWindow(GetCurrentProcessId());// FindWindowA(D3D11Hook::instance()->GetWindowClassName().c_str(), NULL);
            ID3D11DeviceContext *immediateContext = nullptr;
            a1->device->GetImmediateContext(&immediateContext);
            events.on_d3d11_initialized(focusWindow, a1->device, immediateContext);
        });

		auto swapChain = a1->swapChain;

		events.on_present_callback();

		auto result = injectPresent.call(a1);

		return 0;
	});
}

using D3D11CreateDevice_t = HRESULT(WINAPI*)(_In_opt_ IDXGIAdapter *pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
	_In_opt_ const D3D_FEATURE_LEVEL *pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
	_Out_opt_ ID3D11Device **ppDevice, _Out_opt_ D3D_FEATURE_LEVEL *pFeatureLevel, _Out_opt_ ID3D11DeviceContext **ppImmediateContext);


HRESULT _D3D11CreateDevice(_In_opt_ IDXGIAdapter *pAdapter,
	D3D_DRIVER_TYPE     DriverType,
	HMODULE             Software,
	UINT                Flags,
	_In_opt_  const D3D_FEATURE_LEVEL   *pFeatureLevels,
	UINT                FeatureLevels,
	UINT                SDKVersion,
	_Out_opt_       ID3D11Device        **ppDevice,
	_Out_opt_       D3D_FEATURE_LEVEL   *pFeatureLevel,
	_Out_opt_       ID3D11DeviceContext **ppImmediateContext)
{
	// DO STUFF
	HookJC3Present();

	static auto oD3D11CreateDevice = (D3D11CreateDevice_t)(g_originalD3D11CreateDevice);

	HRESULT hr = oD3D11CreateDevice(pAdapter, DriverType, Software, Flags,
		pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

	return hr;
}

D3D11HookEvents &InstallD3D11Hook(bool hotReload)
{
    if (hotReload) {
        HookJC3Present();
        return events;
    }
	g_originalD3D11CreateDevice = (uintptr_t)GetProcAddress(GetModuleHandle(L"d3d11.dll"), "D3D11CreateDevice");
	util::hooking::set_import("D3D11CreateDevice", (uintptr_t)_D3D11CreateDevice);
	return events;
}