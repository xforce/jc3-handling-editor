
#include "jc3_hooking/hooking.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>

#include "d3d11_hook.h"

#define WIN32_NO_STATUS
#include <Windows.h>
#include <d3d11.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include "../deps/imgui/imgui.h"
#include "imgui_impl_dx11.h"

#include <d3dcompiler.h>

#include <jc3/entities/character.h>
#include <jc3/entities/vehicle.h>
#include <jc3/ui/overlay_ui.h>

namespace fs = std::experimental::filesystem;

// NOTE(xforce): Kind of horrible code

WNDPROC g_originalWndProc = NULL;

std::wstring ThisDllDirPath()
{
    std::wstring thisPath = L"";
    WCHAR path[MAX_PATH];
    HMODULE hm;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPWSTR)&ThisDllDirPath, &hm))

    GetModuleFileNameW(hm, path, sizeof(path));
    PathRemoveFileSpecW(path);
    thisPath = std::wstring(path);
    if (!thisPath.empty() &&
        thisPath.at(thisPath.length() - 1) != '\\')
        thisPath += L"\\";

    return thisPath;
}

std::wstring GetProfilesSavePath() {
    auto savePath = fs::path(ThisDllDirPath()).append("profiles");
    if (!fs::exists(savePath)) {
        fs::create_directories(savePath);
    }
    return savePath;
}

std::wstring GetProfileSavePath(std::wstring subDir) {
    auto savePath = fs::path(GetProfilesSavePath()).append(subDir);
    if (!fs::exists(savePath)) {
        fs::create_directories(savePath);
    }
    return savePath;
}

static bool overlayState = false;

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_KEYUP) {
		if (wParam == VK_F8) {
			overlayState = !overlayState;
			if (overlayState) {
				jc3::COverlayUI::ShowCursor();
				ImGui::GetIO().MouseDrawCursor = true;
			}
			else {
				jc3::COverlayUI::HideCursor();
				ImGui::GetIO().MouseDrawCursor = false;
			}
		}
	}

	if (overlayState) {
		if (ImGui_ImplDX11_WndProcHandler(hWnd, msg, wParam, lParam)) {
			return S_OK;
		}
	}

	return CallWindowProc(g_originalWndProc, hWnd, msg, wParam, lParam);
}

uintptr_t GetProcessBaseAddress()
{
    MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    if (Module32First(hSnapshot, &me32))
    {
        CloseHandle(hSnapshot);
        return (uintptr_t)me32.modBaseAddr;
    }

    CloseHandle(hSnapshot);
    return 0;
}

int32_t hook_GetSystemMetrics(int32_t index) {
    return FALSE;
}

void DoCarHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle);
void SetupImGuiStyle2();
void HookZwSetInformationThread();
BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        HookZwSetInformationThread();

        // Load all profiles
        for (auto &p : fs::directory_iterator(GetProfilesSavePath())) {
        }

        std::ofstream meowf(fs::path(GetProfileSavePath(L"TestVehicle")).append("test.json"));
        meowf << "Hello";
        meowf.close();

        // TODO(xforce): Move this to hooking, add some initialize function
        util::hooking::hooking_helpers::SetExecutableAddress(GetProcessBaseAddress());
        util::hooking::ExecutableInfo::instance()->EnsureExecutableInfo();
        auto meow = VirtualAlloc((LPVOID)0x0000000160000000, 0x6000000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        meow = meow;

        HMODULE module = NULL;
        char buffer[MAX_PATH] = { 0 };
        ::GetSystemDirectoryA(buffer, MAX_PATH);
        strcat(buffer, "\\d3d11.dll");
        module = LoadLibraryA(buffer);

		auto &d3d11_hook_events = InstallD3D11Hook();
		d3d11_hook_events.on_d3d11_initialized.connect([](HWND hwnd, ID3D11Device *device, ID3D11DeviceContext *deviceContext) {
			g_originalWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			ImGui_ImplDX11_Init(hwnd, device, deviceContext);
			SetupImGuiStyle2();
		});

		d3d11_hook_events.on_present_callback.connect([]() {
			ImGui_ImplDX11_NewFrame();
			if (overlayState) {
				if (jc3::CCharacter::GetLocalPlayerCharacter()) {
					auto vehicle = jc3::CCharacter::GetLocalPlayerCharacter()->GetVehicle();
					if (vehicle) {
						auto real_vehicle = static_cast<jc3::CVehicle*>(vehicle);
						auto pfxVehicle = static_cast<jc3::CVehicle*>(vehicle)->PfxVehicle;
						if (pfxVehicle && pfxVehicle->GetType() == jc3::PfxType::Car) {
							DoCarHandlingUI(real_vehicle, pfxVehicle);
						}
						else if (pfxVehicle && pfxVehicle->GetType() == jc3::PfxType::MotorBike) {
							// TODO(xforce): MotorBike
						}
					}
				}
			}
			ImGui::Render();
		});

      
        // Want to use it using RDP, yes I do use RDP quite a lot
        util::hooking::set_import("GetSystemMetrics", (uintptr_t)hook_GetSystemMetrics);
    }
    if (fdwReason == DLL_PROCESS_DETACH) {
        ImGui_ImplDX11_Shutdown();
    }
    return TRUE;
}