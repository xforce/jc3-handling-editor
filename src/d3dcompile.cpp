#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

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
