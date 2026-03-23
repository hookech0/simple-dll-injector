#include <windows.h>
#include <TlHelp32.h>

#include "utils.h"
#include "logger.h"



struct ProcessInfo {
	uint32_t pid;
	std::string name;
};

std::vector<ProcessInfo> GetRunningProcesses() {
	std::vector<ProcessInfo> result;

	HANDLE hSnapShot = nullptr;

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE) {
		//printf("[!] CreateToolhelp32Snapshot Failed With Error : %d \n", GetLastError());
		Log(LogLevel::Error, "[ENUM] CreateToolhelp32Snapshot Failed With Error: " + std::to_string(GetLastError()));
		return result;
	}

	PROCESSENTRY32 pe32{};
	pe32.dwSize = sizeof(pe32);

	if (!Process32First(hSnapShot, &pe32)) {
		//printf("[!] Process32First Failed With Error : %d \n", GetLastError());
		Log(LogLevel::Error, "[ENUM] Process32First Failed With Error: " + std::to_string(GetLastError()));
		CloseHandle(hSnapShot);
		return result;
	}

	do {

		if (pe32.th32ProcessID == 0) continue;

		ProcessInfo info;
		info.pid = pe32.th32ProcessID;

		info.pid = pe32.th32ProcessID;
		info.name = WideToUtf8(pe32.szExeFile);

		result.push_back(std::move(info));

	} while (Process32Next(hSnapShot, &pe32));

	CloseHandle(hSnapShot);

	return result;
}