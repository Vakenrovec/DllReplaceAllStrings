// DllReplaceAllStrings.cpp : Определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include <cstdlib>

BOOL ReplaceAllStrings(BYTE *lpLine, SIZE_T stSize) {
	BOOL fChange = FALSE;
	char str1[] = "hello";
	char str2[] = "12345";
	for (BYTE *i = lpLine; i < lpLine + stSize; i++) {
		if (!memcmp(i, str1, strlen(str1))) {
			for (int j = 0; j < strlen(str2); j++, i++) {
				*i = str2[j];
			}
			fChange = TRUE;
			i--;
		}
	}
	return fChange;
}

BOOL WINAPI ReadVirtualMemory(DWORD dwProcessId) {
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (hProcess == NULL) {
		return FALSE;
	}

	SYSTEM_INFO msi;
	GetSystemInfo(&msi);
	LPVOID lpvMinAddress = msi.lpMinimumApplicationAddress;
	LPVOID lpvMaxAddress = msi.lpMaximumApplicationAddress;
	DWORD dwPageSize = msi.dwPageSize;

	MEMORY_BASIC_INFORMATION mbi;
	SIZE_T stNumberOfBytes;
	BYTE *lpLine = (BYTE *)malloc(sizeof(BYTE) * dwPageSize);
	while (lpvMinAddress < lpvMaxAddress) {
		int sz = VirtualQueryEx(hProcess, lpvMinAddress, &mbi, sizeof(mbi));
		if (mbi.State == MEM_COMMIT &&
			(mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE ||
			mbi.Protect == PAGE_WRITECOPY || mbi.Protect == PAGE_EXECUTE_WRITECOPY))
		{
			if (ReadProcessMemory(hProcess, lpvMinAddress, lpLine, dwPageSize, &stNumberOfBytes)) {
				if (ReplaceAllStrings(lpLine, stNumberOfBytes)) {
					WriteProcessMemory(hProcess, lpvMinAddress, lpLine, stNumberOfBytes, &stNumberOfBytes);
				}
			}
		}
		lpvMinAddress = (PBYTE)lpvMinAddress + dwPageSize;
	}
	CloseHandle(hProcess);
	free(lpLine);
	return TRUE;
}

