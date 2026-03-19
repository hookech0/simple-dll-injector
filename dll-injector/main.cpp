#include <windows.h>
#include <stdio.h>
#include <TlHelp32.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "procenum.h"
#include "inject.h"



int wmain(int argc, wchar_t* argv[]) {

	DWORD targetPid = 0;
	DWORD dwProcessId = NULL;
	HANDLE hProcess = NULL;

	if (argc < 2) {
		wprintf(L"[!] Usage: \"%s\" <Complete Dll Payload Path>\n", argv[0]);
		return -1;
	}

	//if (!PrintProcesses()) {
	//	return 1;
	//}

	//printf("\nEnter PID to target: ");
	//scanf_s("%lu", &targetPid);

	//if (!VerifyPID(targetPid)) {
	//	return 1;
	//}

	//if (!GetRemoteProcessHandle(targetPid, &hProcess)) {
	//	return 1;
	//}

	if (!InjectDllToRemoteProcess(hProcess, argv[1])) {
		return 1;
	}

	CloseHandle(hProcess);
	printf("[#] Press <Enter> to Quit...");
	getchar();

}