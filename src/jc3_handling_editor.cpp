
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

#include <jc3/entities/CCharacter.h>
#include <jc3/entities/CVehicle.h>

#include <d3dcompiler.h>

namespace fs = std::experimental::filesystem;

// NOTE(xforce): Kind of horrible code

WNDPROC g_originalWndProc = NULL;

void ShowCursor() {
	auto overlayInstance = *(uintptr_t*)(0x142F38858);
	util::hooking::func_call<void>(0x14470FE60, overlayInstance);
}

void HideCursor() {
	auto overlayInstance = *(uintptr_t*)(0x142F38858);
	util::hooking::func_call<void>(0x14470FE10, overlayInstance);
}

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
				ShowCursor();
				ImGui::GetIO().MouseDrawCursor = true;
			}
			else {
				HideCursor();
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

HRESULT WINAPI D3DCompile(
    LPCVOID                         pSrcData,
    SIZE_T                          SrcDataSize,
    LPCSTR                          pFileName,
    CONST D3D_SHADER_MACRO*         pDefines,
    ID3DInclude*                    pInclude,
    LPCSTR                          pEntrypoint,
    LPCSTR                          pTarget,
    UINT                            Flags1,
    UINT                            Flags2,
    ID3DBlob**                      ppCode,
    ID3DBlob**                      ppErrorMsgs
) {

    static pD3DCompile fnD3DCompile = nullptr;

    // Code based on QT Angle D3DCompiler
    static HMODULE d3dcompiler = 0;
    if (!d3dcompiler) {
        const char *dllNames[] = {
            "d3dcompiler_47.dll",
            "d3dcompiler_46.dll",
            "d3dcompiler_45.dll",
            "d3dcompiler_44.dll",
            "d3dcompiler_43.dll",
            0
        };

        for (int i = 0; const char *name = dllNames[i]; ++i)
        {
            d3dcompiler = LoadLibraryA(name);
            if (d3dcompiler) {
                fnD3DCompile = reinterpret_cast<pD3DCompile>(GetProcAddress(d3dcompiler, "D3DCompile"));
                if (fnD3DCompile) {
                    break;
                }
            }
        }
    }

    if (fnD3DCompile)
    {
        HRESULT hr = S_OK;
        return fnD3DCompile(pSrcData, SrcDataSize, pFileName, pDefines, pInclude, pEntrypoint, pTarget, Flags1, Flags2, ppCode, ppErrorMsgs);
    }

    return S_FALSE;
}


void RenderStuff() {

}
struct ImVec3 { float x, y, z; ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { x = _x; y = _y; z = _z; } };
void imgui_easy_theming(ImVec3 color_for_text, ImVec3 color_for_head, ImVec3 color_for_area, ImVec3 color_for_body, ImVec3 color_for_pops)
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.58f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.95f);
    style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.58f);
    style.Colors[ImGuiCol_Border] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.47f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.21f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_ComboBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.80f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.76f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_Column] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.32f);
    style.Colors[ImGuiCol_ColumnHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
    style.Colors[ImGuiCol_ColumnActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.15f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_CloseButton] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.16f);
    style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.39f);
    style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.43f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(color_for_pops.x, color_for_pops.y, color_for_pops.z, 0.92f);
    style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.73f);
    style.Alpha = 1.0f;
    style.FrameRounding = 0;
    style.WindowRounding = 0;
    style.IndentSpacing = 12.0f;
}

void SetupImGuiStyle2()
{
    static ImVec3 color_for_text = ImVec3(236.f / 255.f, 240.f / 255.f, 241.f / 255.f);
    static ImVec3 color_for_head = ImVec3(41.f / 255.f, 128.f / 255.f, 185.f / 255.f);
    static ImVec3 color_for_area = ImVec3(57.f / 255.f, 79.f / 255.f, 105.f / 255.f);
    static ImVec3 color_for_body = ImVec3(44.f / 255.f, 62.f / 255.f, 80.f / 255.f);
    static ImVec3 color_for_pops = ImVec3(33.f / 255.f, 46.f / 255.f, 60.f / 255.f);
    imgui_easy_theming(color_for_text, color_for_head, color_for_area, color_for_body, color_for_pops);
}

void DoCarHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle) {
    // Car stuff
    auto pfxCar = static_cast<jc3::CPfxCar*>(pfxVehicle);
    ImGui::BulletText("Engine Torque %f", pfxCar->engineTorque);
    ImGui::BulletText("Engine RPM %f", pfxCar->engineRPM);
    ImGui::BulletText("Top Speed %f", pfxCar->topSpeedKph);
    ImGui::Separator();

    ImGui::DragFloat("Top Speed", (float*)((char*)pfxVehicle + 0x3EC));

    ImGui::DragFloat("Drag Coefficient", &real_vehicle->DragCoefficient);
    ImGui::DragFloat("Mass", &real_vehicle->Mass);
    ImGui::DragFloat("Linear Damping", &real_vehicle->LinearDamping);
    ImGui::DragFloat("Angular Damping", &real_vehicle->AngularDamping);
    ImGui::SliderFloat("Gravity Factor", &real_vehicle->GravityFactor, -128, 128);

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Gravity Modifiers")) {
        ImGui::TreePush("Gravity Modifiers");
        ImGui::DragFloat("Gravity Grounded", &pfxCar->someGravityModifiers->gravityMultiplierGrounded);
        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Engine"))
    {
        ImGui::TreePush("Engine");
        auto engine = pfxCar->landVehicleEngine;
        ImGui::Checkbox("Is Clutching", (bool*)&engine->isClutching);
        ImGui::DragFloat("Clutch delay", &engine->clutchDelay);
        ImGui::DragFloat("Clutching Time", &engine->clutchingTime);
        ImGui::DragFloat("Clutch amount", &engine->clutchAmount);
        ImGui::DragFloat("Manual Clutch Engage Timer", &engine->manualClutchEngageTimer);
        ImGui::DragFloat("Source clutch Rpm", &engine->sourceClutchRpm);
        ImGui::DragFloat("Target Clutch Rpm", &engine->targetClutchRpm);
        ImGui::DragFloat("Engine Revs", &engine->engineRevs);
        ImGui::DragFloat("Engine Damage", &engine->engineDamage);
        ImGui::DragFloat("Rev Limiter Magnitude RPM", &engine->revLimiterMagnitudeRPM);
        ImGui::Checkbox("Is Rev Limiting", (bool*)&engine->isRevLimiting);
        ImGui::DragFloat("Full Load Torque", &engine->fullLoadTorque);
        ImGui::DragFloat("Lowest Max Torque", &engine->lowestMaxTorque);
        ImGui::DragFloat("Engine Min Noise", &engine->engineMinNoise);
        ImGui::DragFloat("Engine Damage Noise Scale", &engine->engineDamageNoiseScale);
        ImGui::DragFloat("Engine Max Damage Torque Factor", &engine->engineMaxDamageTorqueFactor);
        ImGui::DragFloat("Min RPM", &engine->minRPM);
        ImGui::DragFloat("Optimal RPM", &engine->optRPM);
        ImGui::DragFloat("Max Torque", &engine->maxTorque);
        ImGui::DragFloat("Torque Factor at Min RPM", &engine->torqueFactorAtMinRPM);
        ImGui::DragFloat("Torque Factor at Max RPM", &engine->torqueFactorAtMaxRPM);
        ImGui::DragFloat("Resistance Factor at Min RPM", &engine->resistanceFactorAtMinRPM);
        ImGui::DragFloat("Resistance Factor at Optimal RPM", &engine->resistanceFactorAtOptRPM);
        ImGui::DragFloat("Resistance Factor at Max RPM", &engine->resistanceFactorAtMaxRPM);
        ImGui::DragFloat("Clutch Slop RPM", &engine->clutchSlipRPM);
        ImGui::DragFloat("Max RPM", &engine->maxRPM);
        ImGui::DragFloat("Overdrive Max RPM", &engine->overdriveMaxRPM);
        ImGui::Checkbox("Overdrive Active", (bool*)&engine->isOverdriveActive);
        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Engine Transmission")) {
        ImGui::TreePush("Engine Transmission");
        auto engineTransmission = pfxCar->landVehicleTransmission;

        ImGui::SliderFloat("Forward Torque Ratio", &engineTransmission->transmissionProperties.forwardTorqueRatio, 0, 128);
        ImGui::SliderFloat("Low Gear Forward Torque Ratio", &engineTransmission->transmissionProperties.lowGearForwardTorqueRatio, 0, 128);
        ImGui::SliderFloat("Max Transmission RPM", &engineTransmission->transmissionProperties.maxTransmissionRPM, 0, 128);
        ImGui::SliderFloat("Max Reverse Transmission RPM", &engineTransmission->transmissionProperties.maxReversingTransmissionRPM, 0, 128);
        ImGui::SliderFloat("Target Cruise RPM", &engineTransmission->transmissionProperties.targetCruiseRPM, 0, 50000);
        ImGui::SliderFloat("Decay Time to Cruise RPM", &engineTransmission->transmissionProperties.decayTimeToCruiseRPM, -10, 128);
        ImGui::SliderFloat("Low Gearing Primary Transmission Ratio", &engineTransmission->transmissionProperties.lowGearingPrimaryTransmissionRatio, -10, 128);
        ImGui::SliderFloat("Downshift RPM", &engineTransmission->transmissionProperties.downshiftRPM, 0, 50000);
        ImGui::SliderFloat("Upshift RPM", &engineTransmission->transmissionProperties.upshiftRPM, 0, 50000);
        ImGui::SliderFloat("Primary Transmission Ratio", &engineTransmission->transmissionProperties.primaryTransmissionRatio, 0, 128);
        for (int i = 0; i < engineTransmission->transmissionProperties.wheelsTorqueRatio.size; ++i) {
            char wheel_text[100];
            sprintf(wheel_text, "Wheel Torque Ratio %d", i);
            ImGui::TreePush(wheel_text);
            ImGui::SliderFloat(wheel_text, &engineTransmission->transmissionProperties.wheelsTorqueRatio.Data[i], 0, 128);
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Suspension")) {
        ImGui::TreePush("Suspension");
        for (int i = 0; i < pfxCar->wheelInfo.size; ++i) {
            auto & suspension = pfxCar->wheelSuspensions[i];
            char wheel_text[100];
            sprintf(wheel_text, "Suspension Wheel %d", i);
            if (ImGui::CollapsingHeader(wheel_text)) {
                ImGui::TreePush(wheel_text);
                ImGui::DragFloat("Suspension Force Mag At Rest", &suspension.suspensionForceMagnitudeAtRest);
                ImGui::DragFloat("Suspension Length At Rest", &suspension.suspensionLengthAtRest);
                if (ImGui::CollapsingHeader("Properties")) {
                    auto properties = suspension.suspensionProperties;
                    ImGui::DragFloat("Antirollbar Strength", &properties->antiRollbarStrength);
                    ImGui::DragFloat("Compression", &properties->compression);
                    ImGui::DragFloat("Length", &properties->length);
                    ImGui::DragFloat("Relaxation", &properties->relaxation);
                    ImGui::DragFloat("Strength", &properties->strength);
                    ImGui::DragFloat("Lateral Tire Force Offset", &properties->lateralTireForceOffset);
                    ImGui::DragFloat("Longtid Tire Force Offset", &properties->longitudinalTireForceOffse);
                    ImGui::DragFloat("Tire drag Force offset", &properties->tireDragForceOffset);
                    ImGui::DragFloat("Hardpoint offset along spring", &properties->hardpointOffsetOnSpring);
                }
                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }

    std::this_thread::yield();

    if (ImGui::CollapsingHeader("Brakes")) {
        ImGui::TreePush("Brakes Front");
        ImGui::Text("Front");
        ImGui::Checkbox("Handbrake", (bool*)&pfxCar->brakesProperties->front.handbrake);
        // Just making sure all the memory is set :D
        if (*(bool*)&pfxCar->brakesProperties->front.handbrake) {
            pfxCar->brakesProperties->front.handbrake = 1;
        }
        else {
            pfxCar->brakesProperties->front.handbrake = 0;
        }
        ImGui::DragFloat("Max Brake Torque", &pfxCar->brakesProperties->front.maxTorque);
        ImGui::DragFloat("Time To Block", &pfxCar->brakesProperties->front.minTimeToBlock);
        ImGui::TreePop();
        ImGui::Separator();
        ImGui::Text("Rear");
        ImGui::TreePush("Brakes Rear");
        ImGui::Checkbox("Handbrake", (bool*)&pfxCar->brakesProperties->rear.handbrake);
        // Just making sure all the memory is set :D
        if (*(bool*)&pfxCar->brakesProperties->rear.handbrake) {
            pfxCar->brakesProperties->rear.handbrake = 1;
        }
        else {
            pfxCar->brakesProperties->rear.handbrake = 0;
        }
        ImGui::DragFloat("Max Brake Torque", &pfxCar->brakesProperties->rear.maxTorque);
        ImGui::DragFloat("Time To Block", &pfxCar->brakesProperties->rear.minTimeToBlock);
        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Aerodynamics")) {
        ImGui::TreePush("Aerodynamic");
        ImGui::DragFloat("Air Density", &pfxCar->carAerodynamics->airDensity);
        ImGui::DragFloat("Frontal Area", &pfxCar->carAerodynamics->frontalArea);
        ImGui::DragFloat("Drag Coefficient", &pfxCar->carAerodynamics->dragCoefficient);
        ImGui::DragFloat("Top Speed Drag Coefficient", &pfxCar->carAerodynamics->topSpeedDragCoefficient);
        ImGui::DragFloat("Lift Coefficient", &pfxCar->carAerodynamics->liftCoefficient);
        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Wheels")) {

        auto DrawWheelInfo = [](const char *id, jc3::WheelInfo * wheelInfo) {
            ImGui::DragFloat("Spin Velocity", &wheelInfo->spinVelocity);
            ImGui::DragFloat("Spin Angle", &wheelInfo->spinAngle);
            ImGui::DragFloat("Side Force", &wheelInfo->sideForce);
            ImGui::DragFloat("Forward Slip Velocity", &wheelInfo->forwardSlipVelocity);
            ImGui::DragFloat("Side Slip Velocity", &wheelInfo->sideSlipVelocity);
            ImGui::DragFloat("Torque", &wheelInfo->torque);
            ImGui::DragFloat("Angular Velocity", &wheelInfo->angularVelocity);
            ImGui::DragFloat("Inv Inertia", &wheelInfo->invInertia);
            ImGui::DragFloat("Slip Angle Deg", &wheelInfo->slipAngleDeg);
            ImGui::DragFloat("Slip Ratio AE", &wheelInfo->slipRatioSAE);
            ImGui::DragFloat("Camber Angle Deg", &wheelInfo->camberAngleDeg);
            ImGui::DragFloat("Lateral Tire Force Offset", &wheelInfo->lateralTireForceOffset);
            ImGui::DragFloat("Longitudinal Tire Force Offset", &wheelInfo->longitudinalTireForceOffset);
            ImGui::DragFloat("Tire Drag Force Offset", &wheelInfo->tireDragForceOffset);
            ImGui::DragFloat("Friction Multiplier", &wheelInfo->wheelFrictionMultiplier);
            ImGui::DragFloat("Drag Multiplier", &wheelInfo->wheelDragMultiplier);
            ImGui::DragFloat("Burnout Friction Multiplier", &wheelInfo->burnoutFrictionMultiplier);
            ImGui::DragFloat("Ground Friction Torque", &wheelInfo->groundFrictionTorque);
            ImGui::DragFloat("Depth of Contact Point Underwater", &wheelInfo->unkown1);
            ImGui::Checkbox("Constrained to Ground", (bool*)&wheelInfo->isConstrainedToGround);
        };

        for (int i = 0; i < pfxCar->wheelInfo.size; ++i) {
            char wheel_text[100];
            sprintf(wheel_text, "Wheel %d", i);
            ImGui::TreePush(wheel_text);
            if (ImGui::CollapsingHeader(wheel_text)) {
                DrawWheelInfo(wheel_text, &pfxCar->wheelInfo.Data[i]);
            }
            ImGui::TreePop();
        }
    }

    //util::hooking::func_call<void>(0x143794F60, real_vehicle);
    util::hooking::func_call<void>(0x1434A64B0, pfxCar);
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
			MessageBoxW(NULL, p.path().wstring().c_str(), L"", 0);
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