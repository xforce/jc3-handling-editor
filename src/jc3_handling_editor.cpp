
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
#include <jc3/spawn/spawn_system.h>

namespace fs = std::experimental::filesystem;

namespace Input
{
static constexpr uintptr_t CMouse_vtbl = 0x1423562B8;
static constexpr uintptr_t CKeyboard_vtbl = 0x142356268;
static constexpr uintptr_t CJoystick_vtbl = 0x1423564B8;
static constexpr uintptr_t CGamePad_vtbl = 0x142356438;

#pragma pack(push, 1)
class IInputDeviceManager
{
public:
    virtual ~IInputDeviceManager() = default;
    virtual void Function00() = 0;

    uint32_t LastUpdateTime;
    bool m_Enabled;
};
#pragma pack(pop)
}

// NOTE(xforce): Kind of horrible code

WNDPROC g_originalWndProc = NULL;
HWND g_Hwnd = NULL;

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
static bool spawnMenuState = false;

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_KEYUP) {
        if (wParam == VK_F8) {
            overlayState = !overlayState;
            if (overlayState || spawnMenuState) {
                jc3::COverlayUI::ShowCursor();
                auto overlayInstance = *(uintptr_t*)(0x142F38858);
                *(int*)(overlayInstance + 0x50) = 2147483646;
                ImGui::GetIO().MouseDrawCursor = true;
            }
            else {
                jc3::COverlayUI::HideCursor();
                jc3::COverlayUI::LockInput(false);
				ImGui::GetIO().MouseDrawCursor = false;
			}
		}

        if (wParam == VK_F9) {
            spawnMenuState = !spawnMenuState;
            if (spawnMenuState || overlayState) {
                jc3::COverlayUI::ShowCursor();
                auto overlayInstance = *(uintptr_t*)(0x142F38858);
                *(int*)(overlayInstance + 0x50) = 2147483646;
                ImGui::GetIO().MouseDrawCursor = true;
            }
            else {
                jc3::COverlayUI::HideCursor();
                jc3::COverlayUI::LockInput(false);
                ImGui::GetIO().MouseDrawCursor = false;
            }
        }
	}

	if (overlayState || spawnMenuState) {
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

namespace ImGui {
/*
tabLabels: name of all tabs involved
tabSize: number of elements
tabIndex: holds the current active tab
tabOrder: optional array of integers from 0 to tabSize-1 that maps the tab label order. If one of the numbers is replaced by -1 the tab label is not visible (closed). It can be read/modified at runtime.

// USAGE EXAMPLE
static const char* tabNames[] = {"First tab","Second tab","Third tab"};
static int tabOrder[] = {0,1,2};
static int tabSelected = 0;
const bool tabChanged = ImGui::TabLabels(tabNames,sizeof(tabNames)/sizeof(tabNames[0]),tabSelected,tabOrder);
ImGui::Text("\nTab Page For Tab: \"%s\" here.\n",tabNames[tabSelected]);
*/

IMGUI_API bool TabLabels(std::vector<std::string> &tabLabels, int tabSize, int &tabIndex, int *tabOrder = NULL) {
    ImGuiStyle& style = ImGui::GetStyle();

    const ImVec2 itemSpacing = style.ItemSpacing;
    const ImVec4 color = style.Colors[ImGuiCol_Button];
    const ImVec4 colorActive = style.Colors[ImGuiCol_ButtonActive];
    const ImVec4 colorHover = style.Colors[ImGuiCol_ButtonHovered];
    const ImVec4 colorText = style.Colors[ImGuiCol_Text];
    style.ItemSpacing.x = 1;
    style.ItemSpacing.y = 1;
    const ImVec4 colorSelectedTab = ImVec4(color.x, color.y, color.z, color.w*0.5f);
    const ImVec4 colorSelectedTabHovered = ImVec4(colorHover.x, colorHover.y, colorHover.z, colorHover.w*0.5f);
    const ImVec4 colorSelectedTabText = ImVec4(colorText.x*0.8f, colorText.y*0.8f, colorText.z*0.8f, colorText.w*0.8f);

    if (tabSize > 0 && (tabIndex < 0 || tabIndex >= tabSize)) {
        if (!tabOrder)  tabIndex = 0;
        else tabIndex = -1;
    }

    float windowWidth = 0.f, sumX = 0.f;
    windowWidth = ImGui::GetWindowWidth() - style.WindowPadding.x - (ImGui::GetScrollMaxY() > 0 ? style.ScrollbarSize : 0.f);

    static int draggingTabIndex = -1; int draggingTabTargetIndex = -1;   // These are indices inside tabOrder
    static ImVec2 draggingtabSize(0, 0);
    static ImVec2 draggingTabOffset(0, 0);

    const bool isMMBreleased = ImGui::IsMouseReleased(2);
    const bool isMouseDragging = ImGui::IsMouseDragging(0, 2.f);
    int justClosedTabIndex = -1, newtabIndex = tabIndex;


    bool selection_changed = false; bool noButtonDrawn = true;
    for (int j = 0, i; j < tabSize; j++)
    {
        i = tabOrder ? tabOrder[j] : j;
        if (i == -1) continue;

        if (sumX > 0.f) {
            sumX += style.ItemSpacing.x;   // Maybe we can skip it if we use SameLine(0,0) below
            sumX += ImGui::CalcTextSize(tabLabels[i].c_str()).x + 2.f*style.FramePadding.x;
            if (sumX > windowWidth) sumX = 0.f;
            else ImGui::SameLine();
        }

        if (i != tabIndex) {
            // Push the style
            style.Colors[ImGuiCol_Button] = colorSelectedTab;
            style.Colors[ImGuiCol_ButtonActive] = colorSelectedTab;
            style.Colors[ImGuiCol_ButtonHovered] = colorSelectedTabHovered;
            style.Colors[ImGuiCol_Text] = colorSelectedTabText;
        }
        // Draw the button
        ImGui::PushID(i);   // otherwise two tabs with the same name would clash.
        if (ImGui::Button(tabLabels[i].c_str())) { selection_changed = (tabIndex != i); newtabIndex = i; }
        ImGui::PopID();
        if (i != tabIndex) {
            // Reset the style
            style.Colors[ImGuiCol_Button] = color;
            style.Colors[ImGuiCol_ButtonActive] = colorActive;
            style.Colors[ImGuiCol_ButtonHovered] = colorHover;
            style.Colors[ImGuiCol_Text] = colorText;
        }
        noButtonDrawn = false;

        if (sumX == 0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line

        if (ImGui::IsItemHoveredRect()) {
            if (tabOrder) {
                // tab reordering
                if (isMouseDragging) {
                    if (draggingTabIndex == -1) {
                        draggingTabIndex = j;
                        draggingtabSize = ImGui::GetItemRectSize();
                        const ImVec2& mp = ImGui::GetIO().MousePos;
                        const ImVec2 draggingTabCursorPos = ImGui::GetCursorPos();
                        draggingTabOffset = ImVec2(
                            mp.x + draggingtabSize.x*0.5f - sumX + ImGui::GetScrollX(),
                            mp.y + draggingtabSize.y*0.5f - draggingTabCursorPos.y + ImGui::GetScrollY()
                        );

                    }
                }
                else if (draggingTabIndex >= 0 && draggingTabIndex < tabSize && draggingTabIndex != j) {
                    draggingTabTargetIndex = j; // For some odd reasons this seems to get called only when draggingTabIndex < i ! (Probably during mouse dragging ImGui owns the mouse someway and sometimes ImGui::IsItemHovered() is not getting called)
                }
            }
        }

    }

    tabIndex = newtabIndex;

    // Draw tab label while mouse drags it
    if (draggingTabIndex >= 0 && draggingTabIndex < tabSize) {
        const ImVec2& mp = ImGui::GetIO().MousePos;
        const ImVec2 wp = ImGui::GetWindowPos();
        ImVec2 start(wp.x + mp.x - draggingTabOffset.x - draggingtabSize.x*0.5f, wp.y + mp.y - draggingTabOffset.y - draggingtabSize.y*0.5f);
        const ImVec2 end(start.x + draggingtabSize.x, start.y + draggingtabSize.y);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const float draggedBtnAlpha = 0.65f;
        const ImVec4& btnColor = style.Colors[ImGuiCol_Button];
        drawList->AddRectFilled(start, end, ImColor(btnColor.x, btnColor.y, btnColor.z, btnColor.w*draggedBtnAlpha), style.FrameRounding);
        start.x += style.FramePadding.x; start.y += style.FramePadding.y;
        const ImVec4& txtColor = style.Colors[ImGuiCol_Text];
        drawList->AddText(start, ImColor(txtColor.x, txtColor.y, txtColor.z, txtColor.w*draggedBtnAlpha), tabLabels[tabOrder[draggingTabIndex]].c_str());

        ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
    }

    // Drop tab label
    if (draggingTabTargetIndex != -1) {
        // swap draggingTabIndex and draggingTabTargetIndex in tabOrder
        const int tmp = tabOrder[draggingTabTargetIndex];
        tabOrder[draggingTabTargetIndex] = tabOrder[draggingTabIndex];
        tabOrder[draggingTabIndex] = tmp;
        //fprintf(stderr,"%d %d\n",draggingTabIndex,draggingTabTargetIndex);
        draggingTabTargetIndex = draggingTabIndex = -1;
    }

    // Reset draggingTabIndex if necessary
    if (!isMouseDragging) draggingTabIndex = -1;

    // Change selected tab when user closes the selected tab
    if (tabIndex == justClosedTabIndex && tabIndex >= 0) {
        tabIndex = -1;
        for (int j = 0, i; j < tabSize; j++) {
            i = tabOrder ? tabOrder[j] : j;
            if (i == -1) continue;
            tabIndex = i;
            break;
        }
    }

    // Restore the style
    style.Colors[ImGuiCol_Button] = color;
    style.Colors[ImGuiCol_ButtonActive] = colorActive;
    style.Colors[ImGuiCol_ButtonHovered] = colorHover;
    style.Colors[ImGuiCol_Text] = colorText;
    style.ItemSpacing = itemSpacing;

    return selection_changed;
}
} // namespace ImGui


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

	bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values, int height_in_items = -1)
	{
		if (values.empty()) { return false; }
		return ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), static_cast<int>(values.size()), height_in_items);
	}
}



#include <jc3/hashes/vehicles.h>
#include <jc3/hashes/weapons.h>
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
nlohmann::json CarSettingsToJson(jc3::CVehicle * vehicle);
void CarSettingsFromJson(jc3::CVehicle * vehicle, nlohmann::json settings_json);
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

void DoCarHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle);
void DoMotorBikeHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle);
void DoBoatHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle);
void DoHelicopterHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle);
void DoPlaneHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle);

void SetupImGuiStyle2();
void HookZwSetInformationThread(bool hotReload);

static std::vector<std::string> vehicleList;
static int lastSelectedVehicle = 0;

static std::unordered_map<std::string, std::vector<std::string>> spawnMenuvehicleNames;
static std::unordered_map<std::string, std::vector<uint32_t>> spawnMenuvehicleHashes;

static std::unordered_map<std::string, std::vector<std::string>> spawnMenuweaponNames;
static std::unordered_map<std::string, std::vector<uint32_t>> spawnMenuweaponHashes;

void DoSpawnMenu() {
    ImGui::Begin("Spawn Menu 435345");

    static int curIndex = 0;
    static bool spawnOnListItemClick = false;
    static int tabSelected = 0;
    static int typeSelected = 0;
    static std::string type;

    if (ImGui::Button("Spawn") && !type.empty()) {
        if (tabSelected == 0) {
            auto modelId = jc3::CSpawnSystem::instance()->GetModelId(spawnMenuvehicleHashes[type][curIndex]);
            jc3::Matrix spawnMatrix;
            jc3::CCharacter::GetLocalPlayerCharacter()->GetTransform(&spawnMatrix);
            spawnMatrix.SetPosition(jc3::CCharacter::GetLocalPlayerCharacter()->AimPositionWeapons);
            jc3::CSpawnSystem::instance()->Spawn(modelId, spawnMatrix, 0x2207C,
                [](jc3::CSpawnSystem::SpawnFactoryRequest* factoryRequest, stl::vector<boost::shared_ptr<jc3::CGameObject>>* spawned_objects, void* userdata) {});
        }
        if (tabSelected == 1) {
            auto modelId = jc3::CSpawnSystem::instance()->GetModelId(spawnMenuweaponHashes[type][curIndex]);
            jc3::Matrix spawnMatrix;
            jc3::CCharacter::GetLocalPlayerCharacter()->GetTransform(&spawnMatrix);
            spawnMatrix.SetPosition(jc3::CCharacter::GetLocalPlayerCharacter()->AimPositionWeapons);
            jc3::CSpawnSystem::instance()->Spawn(modelId, spawnMatrix, 0x2207C,
                [](jc3::CSpawnSystem::SpawnFactoryRequest* factoryRequest, stl::vector<boost::shared_ptr<jc3::CGameObject>>* spawned_objects, void* userdata) {
            
                for (auto object : *spawned_objects)
                {
                    if (object->IsType(jc3::CGameObject::GetClassIdByName("CWeaponBase")))
                    {
                        // Equip the weapon
                        auto m = boost::static_pointer_cast<jc3::CPhysicsGameObject>(object);
                        util::hooking::func_call<void>(0x143AFB9F0, jc3::CCharacter::GetLocalPlayerCharacter(), &m, true);
                    }
                }
            });
        }
    }
    ImGui::SameLine();
    ImGui::Checkbox("Spawn on list item click", &spawnOnListItemClick);

    static std::vector<std::string> tabNames = { "Vehicles", "Weapons" };
    
    const bool tabChanged = ImGui::TabLabels(tabNames, static_cast<int>(tabNames.size()), tabSelected);
    ImGui::TreePush("InnerList");
    if (tabSelected == 0) {
        std::vector<std::string> meow;
        for (auto & w : spawnMenuvehicleNames) {
            meow.push_back(w.first);
        }
        
        const bool tabChanged = ImGui::TabLabels(meow, static_cast<int>(meow.size()), typeSelected);
        auto it = spawnMenuvehicleNames.begin();
        for (auto i = 0; i < typeSelected; ++i) {
            ++it;
        }

        type = it->first;

        ImGui::PushID(it->first.c_str());
        if (ImGui::ListBox("", &curIndex, it->second, 15)) {
            if (spawnOnListItemClick) {
                auto modelId = jc3::CSpawnSystem::instance()->GetModelId(spawnMenuvehicleHashes[it->first][curIndex]);
                jc3::Matrix spawnMatrix;
                jc3::CCharacter::GetLocalPlayerCharacter()->GetTransform(&spawnMatrix);
                spawnMatrix.SetPosition(jc3::CCharacter::GetLocalPlayerCharacter()->AimPositionWeapons);
                jc3::CSpawnSystem::instance()->Spawn(modelId, spawnMatrix, 0x2207C,
                    [](jc3::CSpawnSystem::SpawnFactoryRequest* factoryRequest, stl::vector<boost::shared_ptr<jc3::CGameObject>>* spawned_objects, void* userdata) {});
            }
        }
        ImGui::PopID();
    }
    if (tabSelected == 1) {
        std::vector<std::string> meow;
        for (auto & w : spawnMenuweaponNames) {
            meow.push_back(w.first);
        }

        const bool tabChanged = ImGui::TabLabels(meow, static_cast<int>(meow.size()), typeSelected);
        auto it = spawnMenuweaponNames.begin();
        for (auto i = 0; i < typeSelected; ++i) {
            ++it;
        }

        type = it->first;

        ImGui::PushID(it->first.c_str());
        if (ImGui::ListBox("", &curIndex, it->second, 15)) {
            if (spawnOnListItemClick) {
                auto modelId = jc3::CSpawnSystem::instance()->GetModelId(spawnMenuvehicleHashes[it->first][curIndex]);
                jc3::Matrix spawnMatrix;
                jc3::CCharacter::GetLocalPlayerCharacter()->GetTransform(&spawnMatrix);
                jc3::CSpawnSystem::instance()->Spawn(modelId, spawnMatrix, 0x2207C,
                    [](jc3::CSpawnSystem::SpawnFactoryRequest* factoryRequest, stl::vector<boost::shared_ptr<jc3::CGameObject>>* spawned_objects, void* userdata) {});
            }
        }
        ImGui::PopID();
    }
    ImGui::TreePop();

    ImGui::End();
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        bool hotReload = true;
        if (*(uint32_t*)(0x0000000160000000) == 0) {
            hotReload = false;
        }
        HookZwSetInformationThread(hotReload);

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

        using json = nlohmann::json;
        static json vehicle_hashes = json::parse(jc3::vehicle_hashes);
        assert(vehicle_hashes.is_array() && "Vehicle hashes is not an array");

        // Car stuff
        for (auto &vehicle_js : vehicle_hashes) {
            if (vehicle_js["name"].is_string()) {
                std::string t = vehicle_js["name"];
                vehicleList.emplace_back(t);
                spawnMenuvehicleNames[vehicle_js["_type"]].push_back(t);
                spawnMenuvehicleHashes[vehicle_js["_type"]].push_back(vehicle_js["hash"]);
            }
            else {
                vehicleList.emplace_back("Unknown vehicle name");
            }
        }

        static json weapon_hashes = json::parse(jc3::weapon_hashes);
        assert(weapon_hashes.is_array() && "Vehicle hashes is not an array");

        // Car stuff
        for (auto &weapon_js : weapon_hashes) {
            if (weapon_js["name"].is_string()) {
                std::string t = weapon_js["name"];
                spawnMenuweaponNames[weapon_js["class"]].push_back(t);
                spawnMenuweaponHashes[weapon_js["class"]].push_back(weapon_js["hash"]);
            }
            else {
                vehicleList.emplace_back("Unknown vehicle name");
            }
        }

        // TODO(xforce): Move this to hooking, add some initialize function
        util::hooking::hooking_helpers::SetExecutableAddress(GetProcessBaseAddress());
        util::hooking::ExecutableInfo::instance()->EnsureExecutableInfo();

        HMODULE module = NULL;
        char buffer[MAX_PATH] = { 0 };
        ::GetSystemDirectoryA(buffer, MAX_PATH);
        strcat(buffer, "\\d3d11.dll");
        module = LoadLibraryA(buffer);

		auto &d3d11_hook_events = InstallD3D11Hook(hotReload);
		d3d11_hook_events.on_d3d11_initialized.connect([](HWND hwnd, ID3D11Device *device, ID3D11DeviceContext *deviceContext) {
			g_originalWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
            g_Hwnd = hwnd;
			ImGui_ImplDX11_Init(hwnd, device, deviceContext);
			SetupImGuiStyle2();
            static util::hooking::inject_call<void, uintptr_t*, float, bool, bool> inject(0x14355960C);
            inject.inject([](uintptr_t *meow, float a2, bool enabled, bool has_focus) {
                auto overlayInstance = *(uintptr_t*)(0x142F38858);
                if ((*meow != Input::CGamePad_vtbl && *meow != Input::CKeyboard_vtbl) || *(int*)(overlayInstance + 0x50) != 2147483646) {
                    inject.call(meow, a2, enabled, has_focus);
                }
            });
		});

		d3d11_hook_events.on_present_callback.connect([]() {
			ImGui_ImplDX11_NewFrame();

            if (spawnMenuState) {
                DoSpawnMenu();
            }

			if (overlayState) {
                ImGui::Combo("meow", &lastSelectedVehicle, vehicleList);
                ImGui::SameLine();
				if(ImGui::Button("Spawkn")) {
                    auto &meow = vehicle_hashes[lastSelectedVehicle];
                    if (meow["hash"].is_number()) {
                        auto modelId = jc3::CSpawnSystem::instance()->GetModelId(meow["hash"]);
                        jc3::Matrix spawnMatrix;
                        jc3::CCharacter::GetLocalPlayerCharacter()->GetTransform(&spawnMatrix);
                        spawnMatrix.SetPosition(jc3::CCharacter::GetLocalPlayerCharacter()->AimPositionWeapons);
                        jc3::CSpawnSystem::instance()->Spawn(modelId, spawnMatrix, 0x2207C,
                            [](jc3::CSpawnSystem::SpawnFactoryRequest* factoryRequest, stl::vector<boost::shared_ptr<jc3::CGameObject>>* spawned_objects, void* userdata) {
                            for (auto object : *spawned_objects) {
                                if (object->IsType(jc3::CGameObject::GetClassIdByName("CVehicle"))) {
                                }
                            }
                        });
                    }
				}
                ImGui::Spacing();
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
							auto profile_name = g_profiles[currentVehicleItem.modelName][currentVehicleItem.profileIndex];
							char file_name[256];
							sprintf(file_name, "%s.json", profile_name.c_str());
							std::ifstream testFile(fs::path(GetProfilesSavePath()).append(currentVehicleItem.modelName).append(file_name));
							if (testFile.is_open()) {
								nlohmann::json settings_json;
								testFile >> settings_json;
								testFile.close();
								CarSettingsFromJson(real_vehicle, settings_json);
							}
						}
                        
						ImGui::SameLine();
						if (ImGui::Button("Save")) {
							if (g_profiles[currentVehicleItem.modelName].empty()) {
								ImGui::OpenPopup("Stacked 1f55577f");
								show_save_as = true;
							}
							if (!show_save_as) {
								auto profile_name = g_profiles[currentVehicleItem.modelName][currentVehicleItem.profileIndex];
								fs::create_directories(fs::path(GetProfilesSavePath()).append(currentVehicleItem.modelName));
								char file_name[256];
								sprintf(file_name, "%s.json", profile_name.c_str());
								std::ofstream testFile(fs::path(GetProfilesSavePath()).append(currentVehicleItem.modelName).append(file_name));
								testFile << std::setw(4) << CarSettingsToJson(real_vehicle);
								testFile.close();
							}
						}

						ImGui::SameLine();
						if (ImGui::Button("Save As")) {
							ImGui::OpenPopup("Stacked k1");
							show_save_as = true;
						}

						static bool open = true;
						{
							ImGui::SetNextWindowSize(ImVec2(300, 90), ImGuiSetCond_FirstUseEver);
							if (ImGui::BeginPopupModal("Stacked 1777"))
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
									testFile << std::setw(4) << CarSettingsToJson(real_vehicle);
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
							DoMotorBikeHandlingUI(real_vehicle, pfxVehicle);
						}
						else if (pfxVehicle && pfxVehicle->GetType() == jc3::PfxType::Helicopter) {
                            DoHelicopterHandlingUI(real_vehicle, pfxVehicle);
						}
						else if (pfxVehicle && pfxVehicle->GetType() == jc3::PfxType::Boat) {
                            DoBoatHandlingUI(real_vehicle, pfxVehicle);
						}
						else if (pfxVehicle && pfxVehicle->GetType() == jc3::PfxType::Airplane) {
                            DoPlaneHandlingUI(real_vehicle, pfxVehicle);
                        }
                        else {
                            // RIP
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
        g_originalWndProc = (WNDPROC)SetWindowLongPtr(g_Hwnd, GWLP_WNDPROC, (LONG_PTR)g_originalWndProc);
        ImGui_ImplDX11_Shutdown();
    }
    return TRUE;
}