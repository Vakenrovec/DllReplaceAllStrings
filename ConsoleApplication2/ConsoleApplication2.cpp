#include "pch.h"
#include <windows.h>
#include <tchar.h>

typedef BOOL(WINAPI* PFNReadVirtualMemory) (DWORD);

int main()
{	
	HINSTANCE hLibrary = LoadLibrary(TEXT("DllReplaceAllStrings"));
	if (hLibrary == NULL) {
		_tprintf(TEXT("library wasn't founded"));
		return 0;
	}
	PFNReadVirtualMemory pfnReadVirtualMemory = (PFNReadVirtualMemory)GetProcAddress(hLibrary, "ReadVirtualMemory");
	if (pfnReadVirtualMemory == NULL)
	{
		_tprintf(TEXT("function ReadVirtualMemory wasn't founded"));
		FreeLibrary(hLibrary);
		return 0;
	}
	HWND hFind = FindWindow(NULL, TEXT("C:\\BSUIR\\5 сем\\ОСиСП\\лабы\\check for l3\\Debug\\a1.exe"));
	if (hFind == NULL) {
		_tprintf(TEXT("fail FindWindow\n"));
		FreeLibrary(hLibrary);
		return 0;
	}
	DWORD dwProcessId = 0;
	GetWindowThreadProcessId(hFind, &dwProcessId);
	if (pfnReadVirtualMemory(dwProcessId)) {
		_tprintf(TEXT("true\n"));
	}
	else {
		_tprintf(TEXT("this process %d isn't running\n"), dwProcessId);
	}
	FreeLibrary(hLibrary);
	return 0;
}
