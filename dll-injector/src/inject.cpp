#include <windows.h>
#include <stdio.h>

#include "logger.h"
#include "utils.h"

BOOL InjectDllToRemoteProcess(IN HANDLE hProcess, IN LPWSTR DllName) {

	BOOL bState = TRUE;

	typedef HMODULE(WINAPI* pfnLoadLibraryW)(LPCWSTR);
	LPVOID pAddress = NULL;

	DWORD dwSizeToWrite = lstrlenW(DllName) * sizeof(WCHAR);
	SIZE_T  lpNumberOfBytesWritten = NULL;

	HANDLE hThread = NULL;

	pfnLoadLibraryW LoadLibraryWAddr = (pfnLoadLibraryW)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	if (LoadLibraryWAddr == NULL) {
		Log(LogLevel::Error, "GetProcAddress Failed With Error: " + std::to_string(GetLastError()));
		bState = FALSE; goto _EndOfFunc;
	}

	pAddress = VirtualAllocEx(hProcess, NULL, dwSizeToWrite, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (pAddress == NULL) {
		Log(LogLevel::Error, "VirtualAllocEx Failed With Error: " + std::to_string(GetLastError()));
		bState = FALSE; goto _EndOfFunc;
	}

	if (!WriteProcessMemory(hProcess, pAddress, DllName, dwSizeToWrite, &lpNumberOfBytesWritten) || lpNumberOfBytesWritten != dwSizeToWrite) {
		Log(LogLevel::Error, "WriteProcessMemory Failed With Error: " + std::to_string(GetLastError()));
		bState = FALSE; goto _EndOfFunc;
	}

	Log(LogLevel::Success, "Successfully Wrote " + std::to_string(dwSizeToWrite) + " Bytes");

	hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryWAddr, pAddress, NULL, NULL);
	if (hThread == NULL) {
		Log(LogLevel::Error, "CreateRemoteThread Failed With Error: " + std::to_string(GetLastError()));
		bState = FALSE; goto _EndOfFunc;
	}

	Log(LogLevel::Success, "DLL Written and Executed From Remote Process");

_EndOfFunc:
	if (hThread) {
		CloseHandle(hThread);
	}
	return bState;
}