#include <windows.h>
#include <string>
#include <commdlg.h>

#pragma comment(lib, "comdlg32.lib")


std::string WideToUtf8(const std::wstring& wide) {
	if (wide.empty()) return "";
	int len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string result(len, 0);
	WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &result[0], len, nullptr, nullptr);
	result.resize(len - 1);
	return result;
}


BOOL selectFile(std::wstring& selectedFile) {

	static wchar_t filePath[MAX_PATH] = L"";
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"DLL Files\0*.dll\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrTitle = L"Select DLL";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileName(&ofn)) {
		selectedFile = ofn.lpstrFile;
		return true;
	}

	return false;
}