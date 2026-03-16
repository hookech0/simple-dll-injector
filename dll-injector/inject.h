#pragma once
#include <windows.h>

BOOL InjectDllToRemoteProcess(IN HANDLE hProcess, IN LPWSTR DllName);