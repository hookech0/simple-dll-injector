#pragma once

#include <windows.h>


BOOL VerifyPID(IN DWORD dwPID);

BOOL GetRemoteProcessHandle(IN DWORD dwPID, OUT HANDLE* hProcess);