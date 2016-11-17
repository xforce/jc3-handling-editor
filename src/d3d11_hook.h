#pragma once

#include <d3d11.h>
#include <Windows.h>
#include <ksignals.h>

struct D3D11HookEvents
{
	ksignals::Event<void, HWND, ID3D11Device*, ID3D11DeviceContext*> on_d3d11_initialized;
	ksignals::Event<> on_present_callback;
	ksignals::Event<> meow3;
	ksignals::Event<> meow4;
};

D3D11HookEvents &InstallD3D11Hook();