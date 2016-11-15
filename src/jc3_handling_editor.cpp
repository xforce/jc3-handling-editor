
#include "jc3_hooking/hooking.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#define WIN32_NO_STATUS
#include <Windows.h>
#include <d3d11.h>
#include <TlHelp32.h>

#include <Winternl.h>

#include "../deps/imgui/imgui.h"
#include "imgui_impl_dx11.h"

#include <jc3/entities/CCharacter.h>
#include <jc3/entities/CVehicle.h>

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth

#include <d3dcompiler.h>

WNDPROC g_originalWndProc = NULL;

void ShowCursor() {
    auto overlayInstance = *(uintptr_t*)(0x142F38858);
    util::hooking::func_call<void>(0x14470FE60, overlayInstance);
}

void HideCursor() {
    auto overlayInstance = *(uintptr_t*)(0x142F38858);
    util::hooking::func_call<void>(0x14470FE10, overlayInstance);
}

bool overlayState = false;

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

static void* origQIP = nullptr;
typedef NTSTATUS(*ZwSetInformationThreadType)(HANDLE ThreadHandle, THREADINFOCLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength);

NTSTATUS ZwSetInformationThreadHook(HANDLE ThreadHandle, THREADINFOCLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength)
{
    // Don't hide from debugger
    if (ThreadInformationClass == 0x11)
    {
        return STATUS_SUCCESS;
    }
    else
    {
        return ((ZwSetInformationThreadType)origQIP)(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength);
    }
}

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma pack(push, 1)
const ULONG kMmovR10EcxMovEax = 0xB8D18B4C;
const USHORT kSyscall = 0x050F;
const BYTE kRetNp = 0xC3;
const ULONG64 kMov1 = 0x54894808244C8948;
const ULONG64 kMov2 = 0x4C182444894C1024;
const ULONG kMov3 = 0x20244C89;
const USHORT kTestByte = 0x04F6;
const BYTE kPtr = 0x25;
const BYTE kRet = 0xC3;
const USHORT kJne = 0x0375;

// Service code for 64 bit systems.
struct ServiceEntry {
    // This struct contains roughly the following code:
    // 00 mov     r10,rcx
    // 03 mov     eax,52h
    // 08 syscall
    // 0a ret
    // 0b xchg    ax,ax
    // 0e xchg    ax,ax
    ULONG mov_r10_rcx_mov_eax;  // = 4C 8B D1 B8
    ULONG service_id;
    USHORT syscall;             // = 0F 05
    BYTE ret;                   // = C3
    BYTE pad;                   // = 66
    USHORT xchg_ax_ax1;         // = 66 90
    USHORT xchg_ax_ax2;         // = 66 90
};

// Service code for 64 bit Windows 8.
struct ServiceEntryW8 {
    // This struct contains the following code:
    // 00 48894c2408      mov     [rsp+8], rcx
    // 05 4889542410      mov     [rsp+10], rdx
    // 0a 4c89442418      mov     [rsp+18], r8
    // 0f 4c894c2420      mov     [rsp+20], r9
    // 14 4c8bd1          mov     r10,rcx
    // 17 b825000000      mov     eax,25h
    // 1c 0f05            syscall
    // 1e c3              ret
    // 1f 90              nop
    ULONG64 mov_1;              // = 48 89 4C 24 08 48 89 54
    ULONG64 mov_2;              // = 24 10 4C 89 44 24 18 4C
    ULONG mov_3;                // = 89 4C 24 20
    ULONG mov_r10_rcx_mov_eax;  // = 4C 8B D1 B8
    ULONG service_id;
    USHORT syscall;             // = 0F 05
    BYTE ret;                   // = C3
    BYTE nop;                   // = 90
};

// Service code for 64 bit systems with int 2e fallback.
struct ServiceEntryWithInt2E {
    // This struct contains roughly the following code:
    // 00 4c8bd1           mov     r10,rcx
    // 03 b855000000       mov     eax,52h
    // 08 f604250803fe7f01 test byte ptr SharedUserData!308, 1
    // 10 7503             jne [over syscall]
    // 12 0f05             syscall
    // 14 c3               ret
    // 15 cd2e             int 2e
    // 17 c3               ret
    ULONG mov_r10_rcx_mov_eax;  // = 4C 8B D1 B8
    ULONG service_id;
    USHORT test_byte;           // = F6 04
    BYTE ptr;                   // = 25
    ULONG user_shared_data_ptr;
    BYTE one;                   // = 01
    USHORT jne_over_syscall;    // = 75 03
    USHORT syscall;             // = 0F 05
    BYTE ret;                   // = C3
    USHORT int2e;               // = CD 2E
    BYTE ret2;                  // = C3
};

// Service code for 64 bit systems with int 2e fallback.
struct PatchCode {
    // This struct contains roughly the following code:
    uint16_t mov_rax;
    uint64_t ptr;
    uint16_t jmp_rax;
};

// We don't have an internal thunk for x64.
struct ServiceFullThunk {
    union {
        ServiceEntry original;
        ServiceEntryW8 original_w8;
        ServiceEntryWithInt2E original_int2e_fallback;
    };
};
#pragma pack(pop)

bool IsService(const void *source)
{
    const ServiceEntry* service = reinterpret_cast<const ServiceEntry*>(source);
    return (kMmovR10EcxMovEax == service->mov_r10_rcx_mov_eax && kSyscall == service->syscall && kRetNp == service->ret);
}

bool IsServiceW8(const void *source)
{
    const ServiceEntryW8* service = reinterpret_cast<const ServiceEntryW8*>(source);
    return (kMmovR10EcxMovEax == service->mov_r10_rcx_mov_eax &&
        kMov1 == service->mov_1 && kMov2 == service->mov_2 &&
        kMov3 == service->mov_3);
}

bool IsServiceWithInt2E(const void *source)
{
    const ServiceEntryWithInt2E* service = reinterpret_cast<const ServiceEntryWithInt2E*>(source);
    return (kMmovR10EcxMovEax == service->mov_r10_rcx_mov_eax &&
        kTestByte == service->test_byte && kPtr == service->ptr &&
        kJne == service->jne_over_syscall && kSyscall == service->syscall &&
        kRet == service->ret && kRet == service->ret2);
}

void HookZwSetInformationThread()
{
    ServiceFullThunk *code = (ServiceFullThunk*)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetInformationThread");

    assert(code != 0);

    size_t size = 0;

    if (IsService(code))
    {
        size = sizeof ServiceEntry;
    }
    else if (IsServiceW8(code))
    {
        size = sizeof ServiceEntryW8;
    }
    else if (IsServiceWithInt2E(code))
    {
        size = sizeof ServiceEntryWithInt2E;
    }

    origQIP = (ServiceFullThunk*)VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    memcpy(origQIP, code, size);

    DWORD oldProtect;
    VirtualProtect(code, size, PAGE_EXECUTE_READWRITE, &oldProtect);

    PatchCode patch;
    patch.mov_rax = 0xB848;
    patch.ptr = (uint64_t)ZwSetInformationThreadHook;
    patch.jmp_rax = 0xE0FF;

    memcpy(code, &patch, sizeof(PatchCode));
}

uintptr_t g_originalD3D11CreateDevice = 0;
ID3D11Device *g_device = nullptr;
ID3D11DeviceContext *g_deviceContext = nullptr;
IDXGISwapChain *g_swapChain = nullptr;
HWND g_JC3Hwnd = NULL;


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

    static auto injectPresent = util::hooking::inject_call<int64_t, AvalancheDXGIDevice*>(0x1432E0071);
    injectPresent.inject([](AvalancheDXGIDevice *a1) -> int64_t {
        auto swapChain = a1->swapChain;

        ImGui_ImplDX11_NewFrame();
        if (jc3::CCharacter::GetLocalPlayerCharacter()) {
            auto vehicle = jc3::CCharacter::GetLocalPlayerCharacter()->GetVehicle();
            if (vehicle) {
                auto real_vehicle = static_cast<jc3::CVehicle*>(vehicle);
                auto pfxVehicle = static_cast<jc3::CVehicle*>(vehicle)->PfxVehicle;
                if (pfxVehicle && pfxVehicle->GetType() == 4) {
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
            }
        }

        ImGui::Render();

        auto result = injectPresent.call(a1);

        return 0;
    });
}

using D3D11CreateDevice_t = HRESULT(WINAPI*)(_In_opt_ IDXGIAdapter *pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
    _In_opt_ const D3D_FEATURE_LEVEL *pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
    _Out_opt_ ID3D11Device **ppDevice, _Out_opt_ D3D_FEATURE_LEVEL *pFeatureLevel, _Out_opt_ ID3D11DeviceContext **ppImmediateContext);

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

    // Swapchain ptr will be grapped from present func

    g_device = *ppDevice;
    g_deviceContext = *ppImmediateContext;


    HWND focusWindow = FindTopWindow(GetCurrentProcessId());// FindWindowA(D3D11Hook::instance()->GetWindowClassName().c_str(), NULL);
    if (focusWindow)
    {
        g_JC3Hwnd = focusWindow;
    }

    g_originalWndProc = (WNDPROC)SetWindowLongPtr(g_JC3Hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

    ImGui_ImplDX11_Init(g_JC3Hwnd, g_device, g_deviceContext);
    SetupImGuiStyle2();

    return hr;
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

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        HookZwSetInformationThread();

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


        g_originalD3D11CreateDevice = (uintptr_t)GetProcAddress(GetModuleHandle(L"d3d11.dll"), "D3D11CreateDevice");
        LoadLibraryA("User32.dll");
        util::hooking::set_import("D3D11CreateDevice", (uintptr_t)_D3D11CreateDevice);

        // Want to use it using RDP, yes I do use RDP quite a lot
        util::hooking::set_import("GetSystemMetrics", (uintptr_t)hook_GetSystemMetrics);
    }
    if (fdwReason == DLL_PROCESS_DETACH) {
        ImGui_ImplDX11_Shutdown();
    }
    return TRUE;
}