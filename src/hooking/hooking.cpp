#include <Winsock2.h>
#include <Windows.h>
#include <cstdint>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include <string>
#include "hooking.h"

namespace hooking
{
	namespace hooking_helpers
	{
		static uintptr_t executableAddress = 0;
		static const uintptr_t hookSectionOffset = 0x20000000;

		extern "C" void SetExecutableAddress(uintptr_t address)
		{
			executableAddress = address;
		}

		extern "C" uintptr_t GetExecutableAddress()
		{
			return executableAddress;
		}

		extern "C" void* AllocInHookSection(size_t size)
		{
			static bool firstCall = true;

			auto addr = executableAddress + hookSectionOffset;
			if (firstCall)
			{
				DWORD oldProtect;
				VirtualProtect((LPVOID)(addr), size, PAGE_READWRITE, &oldProtect);
				*(uint32_t*)(addr) = sizeof(uint32_t);
				firstCall = false;
			}

			auto code = (LPVOID)(addr + *(uint32_t*)(addr));
			DWORD oldProtect;
			VirtualProtect(code, size, PAGE_EXECUTE_READWRITE, &oldProtect);
			*(uint32_t*)(addr) += static_cast<uint32_t>(size);
			return code;
		}
	}
}