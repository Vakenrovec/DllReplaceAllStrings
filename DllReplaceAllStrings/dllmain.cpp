#include "stdafx.h"
#include <tchar.h>

__declspec(dllexport) BOOL WINAPI ReadVirtualMemory(DWORD);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		_tprintf(TEXT("1\n"));
		ReadVirtualMemory(GetCurrentProcessId());
		break;
    case DLL_THREAD_ATTACH:
		_tprintf(TEXT("2\n"));
		break;
    case DLL_THREAD_DETACH:
		_tprintf(TEXT("3\n"));
		break;
    case DLL_PROCESS_DETACH:
		_tprintf(TEXT("4\n"));
        break;
    }
    return TRUE;
}

