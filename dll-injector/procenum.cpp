#include <windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include <string>
#include <vector>


struct ProcessInfo {
	uint32_t pid;
	std::string name;
};



std::vector<ProcessInfo> GetRunningProcesses() {
	std::vector<ProcessInfo> result;

	HANDLE hSnapShot = nullptr;

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE) {
		printf("[!] CreateToolhelp32Snapshot Failed With Error : %d \n", GetLastError());
		return result;
	}

	PROCESSENTRY32 pe32{};
	pe32.dwSize = sizeof(pe32);

	if (!Process32First(hSnapShot, &pe32)) {
		printf("[!] Process32First Failed With Error : %d \n", GetLastError());
		CloseHandle(hSnapShot);
		return result;
	}

	do {

		if (pe32.th32ProcessID == 0) continue;

		ProcessInfo info;
		info.pid = pe32.th32ProcessID;

		int len = WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, nullptr, 0, nullptr, nullptr);
		if (len <= 0) {
			info.name = "<conversion failed>";
			result.push_back(std::move(info));
			continue;
		}

		info.name.resize(len);

		WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, &info.name[0], len, nullptr, nullptr);

		info.name.resize(len - 1);

		result.push_back(std::move(info));
	} while (Process32Next(hSnapShot, &pe32));

	CloseHandle(hSnapShot);

	return result;
}




//BOOL PrintProcesses() {
//
//
//	do {
//		dwNumberOfProcesses++;
//
//		//wprintf(L"%s : %lu\n", pe32.szExeFile, pe32.th32ProcessID);
//
//	} while (Process32Next(hSnapShot, &pe32));
//
//	//printf("\n[i] %lu processes found\n", dwNumberOfProcesses);
//
//_EndOfFunc:
//	if (hSnapShot != NULL)
//		CloseHandle(hSnapShot);
//	return TRUE;
//}