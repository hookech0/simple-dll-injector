#include <windows.h>
#include <stdio.h>
#include <TlHelp32.h>

// Verify process id exists

BOOL VerifyPID(IN DWORD dwPID) {

	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnapShot = NULL;

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hSnapShot == INVALID_HANDLE_VALUE) {
		printf("[!] CreateToolhelp32Snapshot Failed With Error : %d \n", GetLastError());
		goto _EndOfFunc;
	}

	if (!Process32First(hSnapShot, &pe32)) {
		printf("[!] Process32First Failed With Error : %d \n", GetLastError());
		goto _EndOfFunc;
	}

	do {
		if (pe32.th32ProcessID == dwPID) {

			printf("[+] Found target process: %ls | %lu\n", pe32.szExeFile, pe32.th32ProcessID);
			return TRUE; goto _EndOfFunc;
		}

	} while (Process32Next(hSnapShot, &pe32));

_EndOfFunc:
	if (hSnapShot != NULL)
		CloseHandle(hSnapShot);
	return TRUE;
}


// Get Handle to process
// Merge w/ verify?

BOOL GetRemoteProcessHandle(IN DWORD dwPID, OUT HANDLE* hProcess) {

	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnapShot = NULL;

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hSnapShot == INVALID_HANDLE_VALUE) {
		printf("[!] CreateToolhelp32Snapshot Failed With Error : %d \n", GetLastError());
		goto _EndOfFunc;
	}

	if (!Process32First(hSnapShot, &pe32)) {
		printf("[!] Process32First Failed With Error : %d \n", GetLastError());
		goto _EndOfFunc;
	}

	printf("[i] Obtaining a handle to the process...\n");

	do {
		if (pe32.th32ProcessID == dwPID) {

			*hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);

			if (*hProcess == NULL) {
				printf("\t[!] OpenProcess Failed With Error: %d\n", GetLastError());
				break;
			}
		}
	} while (Process32Next(hSnapShot, &pe32));

	printf("[i] Obtained handle to %ls\n", pe32.szExeFile);
	printf("\t- 0x%p\n", hProcess);

_EndOfFunc:
	if (hSnapShot != NULL)
		CloseHandle(hSnapShot);
	if (*hProcess == NULL)
		return FALSE;
	return TRUE;
}