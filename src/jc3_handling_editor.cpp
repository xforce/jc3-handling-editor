
#include "jc3_hooking/hooking.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <string>
#include <unordered_map>

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

std::unordered_map<std::string, std::vector<std::string>> g_profiles;

std::string ThisDllDirPath()
{
    std::string thisPath = "";
    char path[MAX_PATH];
    HMODULE hm;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPWSTR)&ThisDllDirPath, &hm))

    GetModuleFileNameA(hm, path, sizeof(path));
    PathRemoveFileSpecA(path);
    thisPath = std::string(path);
    if (!thisPath.empty() &&
        thisPath.at(thisPath.length() - 1) != '\\')
        thisPath += "\\";

    return thisPath;
}

std::string GetProfilesSavePath() {
    auto savePath = fs::path(ThisDllDirPath()).append("profiles");
    if (!fs::exists(savePath)) {
        fs::create_directories(savePath);
    }
    return savePath.string();
}

std::string GetProfileSavePath(std::string subDir) {
    auto savePath = fs::path(GetProfilesSavePath()).append(subDir);
    if (!fs::exists(savePath)) {
        fs::create_directories(savePath);
    }
    return savePath.string();
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


namespace ImGui
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	bool Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), static_cast<int>(values.size()));
	}

	bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), static_cast<int>(values.size()));
	}

}

#include <jc3/hashes/vehicles.h>
#include <json.hpp>

struct VehicleUIItem
{
	int profileIndex = 0;
	std::string modelName;
	void Reset() {
		profileIndex = 0;
		modelName.clear();
	}
};

static VehicleUIItem currentVehicleItem;

jc3::CGameObject * lastVehicle = nullptr;

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))
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
			if (fs::is_directory(p.path())) {
				auto &m = g_profiles[p.path().filename().string()];
				for (auto &f : fs::directory_iterator(p)) {
					if (f.path().extension() == ".json") {
						auto pp = f.path();
						auto mm = pp.replace_extension("");
						m.push_back(mm.filename().string());
					}
				}
			}
        }

        std::ofstream meowf(fs::path(GetProfileSavePath("TestVehicle")).append("test.json"));
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

					// We don't want it to reset if you get out of the current vehicle and then get back into the same vehicle
					// TOOD(xforce): Improve this so it stores it persistent for the vehicle you were in
					if (vehicle && lastVehicle != vehicle) {
						lastVehicle = vehicle;
						currentVehicleItem.Reset();
						if (vehicle) {
							auto real_vehicle = static_cast<jc3::CVehicle*>(vehicle);

							auto name_hash = real_vehicle->GetNameHash();

							using json = nlohmann::json;
							static json vehicle_hashes = json::parse(jc3::vehicle_hashes);
							assert(vehicle_hashes.is_array() && "Vehicle hashes is not an array");

							// Car stuff
							auto hash = real_vehicle->GetNameHash();

							std::string modelName;
							for (auto &vehicle_js : vehicle_hashes) {
								if (vehicle_js["hash"].is_number() && static_cast<uint32_t>(vehicle_js["hash"]) == hash) {
									std::string t = vehicle_js["model_name"];
									modelName = t;
								}
							}
							if (modelName.empty()) {
								modelName = std::to_string(hash);
							}
							currentVehicleItem.modelName = modelName;
						}
					}

					if (vehicle) {
						auto real_vehicle = static_cast<jc3::CVehicle*>(vehicle);

						bool show_save_as = false;

						if (ImGui::Combo("meow", &currentVehicleItem.profileIndex, g_profiles[currentVehicleItem.modelName])) {
							// TODO(xforce): Load the new settings
						}
						ImGui::SameLine();
						if (ImGui::Button("Save")) {
							if (g_profiles[currentVehicleItem.modelName].empty()) {
								ImGui::OpenPopup("Stacked 1");
								show_save_as = true;
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Save As")) {
							ImGui::OpenPopup("Stacked 1");
							show_save_as = true;
						}

						static bool open = true;
						{
							ImGui::SetNextWindowSize(ImVec2(300, 90), ImGuiSetCond_FirstUseEver);
							if (ImGui::BeginPopupModal("Stacked 1"))
							{
								static char buf[256] = {0};
								ImGui::InputText("Name", buf, sizeof(buf));
								if (ImGui::Button("Save")) {
									if (std::find(std::begin(g_profiles[currentVehicleItem.modelName]), 
													std::end(g_profiles[currentVehicleItem.modelName]), 
													buf) == std::end(g_profiles[currentVehicleItem.modelName])) {
										g_profiles[currentVehicleItem.modelName].emplace_back(buf);
									}

									auto index = std::distance(std::begin(g_profiles[currentVehicleItem.modelName]), std::find(std::begin(g_profiles[currentVehicleItem.modelName]),
										std::end(g_profiles[currentVehicleItem.modelName]),
										buf));
									currentVehicleItem.profileIndex = static_cast<int>(index);

									fs::create_directories(fs::path(GetProfilesSavePath()).append(currentVehicleItem.modelName));
									char file_name[256];
									sprintf(file_name, "%s.json", buf);
									std::ofstream testFile(fs::path(GetProfilesSavePath()).append(currentVehicleItem.modelName).append(file_name));
									testFile.close();
									ImGui::CloseCurrentPopup();
								}
								ImGui::SameLine();
								if (ImGui::Button("Close"))
									ImGui::CloseCurrentPopup();
								ImGui::EndPopup();
							}
						}

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