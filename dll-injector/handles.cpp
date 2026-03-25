#include <windows.h>
#include <stdio.h>
#include <TlHelp32.h>

#include "logger.h"
#include "utils.h"

BOOL GetRemoteProcessHandle(IN DWORD dwPID, OUT HANDLE* hProcess) {

	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnapShot = NULL;

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hSnapShot == INVALID_HANDLE_VALUE) {
		//printf("[!] CreateToolhelp32Snapshot Failed With Error : %d \n", GetLastError());
		Log(LogLevel::Error, "[HANDLE] CreateToolhelp32Snapshot Failed With Error: " + std::to_string(GetLastError()));
		goto _EndOfFunc;
	}

	if (!Process32First(hSnapShot, &pe32)) {
		//printf("[!] Process32First Failed With Error : %d \n", GetLastError());
		Log(LogLevel::Error, "[HANDLE] CreateToolhelp32Snapshot Failed With Error: " + std::to_string(GetLastError()));
		goto _EndOfFunc;
	}

	//printf("[i] Obtaining a handle to the process...\n");

	do {
		if (pe32.th32ProcessID == dwPID) {

			*hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);

			if (*hProcess == NULL) {
				//printf("\t[!] OpenProcess Failed With Error: %d\n", GetLastError());
				Log(LogLevel::Error, "[HANDLE] OpenProcess Failed With Error: " + std::to_string(GetLastError()));
				break;
			}

			Log(LogLevel::Info, "Obtained handle to: [" + std::to_string(pe32.th32ProcessID) + "] " + WideToUtf8(pe32.szExeFile));
			break;
		}
	} while (Process32Next(hSnapShot, &pe32));


_EndOfFunc:
	if (hSnapShot != NULL)
		CloseHandle(hSnapShot);
	if (*hProcess == NULL)
		return FALSE;
	return TRUE;
}