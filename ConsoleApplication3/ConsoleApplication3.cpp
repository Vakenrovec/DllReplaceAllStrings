#include "pch.h"
#include <windows.h>
#include <tchar.h>
#include <TlHelp32.h>

BOOL WINAPI InjectLib(DWORD dwProcessId, PCTSTR psLibFile) {
	BOOL fOk = FALSE;	
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	PTSTR psLibFileRemote = NULL;
	__try {
		hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, dwProcessId);
		if (hProcess == NULL) {
			_tprintf(TEXT("this process %d isn't running\n"), dwProcessId);
			__leave;
		}

		int iNumberOfBytes = (_tcslen(psLibFile) + 1) * sizeof(TCHAR);
		psLibFileRemote = (PTSTR)VirtualAllocEx(hProcess, NULL, iNumberOfBytes, MEM_COMMIT, PAGE_READWRITE);
		if (psLibFileRemote == NULL) {
			_tprintf(TEXT("failed to allocate memory\n"));
			__leave;
		}

		if (!WriteProcessMemory(hProcess, psLibFileRemote, (PVOID)psLibFile, iNumberOfBytes, NULL)) {
			_tprintf(TEXT("failed to write memory\n"));
			__leave;
		}

#ifdef UNICODE
		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
			GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
#else
		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
			GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryA");
#endif // !UNICODE
		if (pfnThreadRtn == NULL) {
			_tprintf(TEXT("failed to GetProcAdress\n"));
			__leave;
		}

		hThread = CreateRemoteThread(hProcess, NULL, 0,
			pfnThreadRtn, psLibFileRemote, 0, NULL);
		if (hThread == NULL) {
			_tprintf(TEXT("failed to CreateRemoteThread\n"));
			__leave;
		}

		WaitForSingleObject(hThread, INFINITE);

		fOk = TRUE;
	}
	__finally {
		if (psLibFileRemote != NULL)
			VirtualFreeEx(hProcess, psLibFileRemote, 0, MEM_RELEASE);
		if (hThread != NULL)
			CloseHandle(hThread);
		if (hProcess != NULL)
			CloseHandle(hProcess);
	}
	return fOk;
}

BOOL WINAPI EjectLib(DWORD dwProcessId, PCTSTR psLibFile) {
	BOOL fOk = FALSE; 
	HANDLE hthSnapshot = NULL;
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	__try {
		hthSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
		if (hthSnapshot == NULL) {
			_tprintf(TEXT("failed to Snapshot\n"));
			__leave;
		}

		MODULEENTRY32W me = { sizeof(me) };
		BOOL fFound = FALSE;
		BOOL fMoreMods = Module32FirstW(hthSnapshot, &me);
		for (; fMoreMods; fMoreMods = Module32NextW(hthSnapshot, &me)) {
			if (fFound = (lstrcmpiW(me.szModule, psLibFile) == 0) || (lstrcmpiW(me.szExePath, psLibFile) == 0)) {
				break;
			}
		}
		if (!fFound) {
			_tprintf(TEXT("failed to fFound\n"));
			__leave;
		}

		hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION, FALSE, dwProcessId);
		if (hProcess == NULL) {
			_tprintf(TEXT("failed to OpenProcess\n"));
			__leave;
		}

		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
			GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "FreeLibrary");
		if (pfnThreadRtn == NULL) {
			_tprintf(TEXT("failed to GetProcAddress\n"));
			__leave;
		}

		hThread = CreateRemoteThread(hProcess, NULL, 0,
			pfnThreadRtn, me.hModule, 0, NULL);
		if (hThread == NULL) {
			_tprintf(TEXT("failed to CreateRemoteThread\n"));
			__leave;
		}

		WaitForSingleObject(hThread, INFINITE);

		fOk = TRUE; 
	}
	__finally { 
		if (hthSnapshot != NULL)
			CloseHandle(hthSnapshot);
		if (hThread != NULL)
			CloseHandle(hThread);
		if (hProcess != NULL)
			CloseHandle(hProcess);
	}
	return(fOk);
}

int main()
{	
	TCHAR psLibFile[MAX_PATH];
	GetModuleFileName(NULL, psLibFile, sizeof(psLibFile));
	_tcscpy_s(_tcsrchr(psLibFile, TEXT('\\')) + 1, MAX_PATH - _tcslen(psLibFile) - 1, TEXT("DllReplaceAllStrings.dll"));
	//_tprintf(TEXT("%s\n"), psLibFile);
	
	HWND hFind = FindWindow(NULL, TEXT("C:\\BSUIR\\5 сем\\ОСиСП\\лабы\\check for l3\\Debug\\a1.exe"));
	if (hFind == NULL) {
		_tprintf(TEXT("fail FindWindow\n"));
	}
	DWORD dwProcessId = 0;
	GetWindowThreadProcessId(hFind, &dwProcessId);
	
	if (!InjectLib(dwProcessId, psLibFile)) {
		_tprintf(TEXT("failed to InjectLib\n"));
	}
	if (!EjectLib(dwProcessId, psLibFile)) {
		_tprintf(TEXT("failed to EjectLib\n"));
	}	
	return 0;
}
