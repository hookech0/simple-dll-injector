#pragma once

#include <string>


std::string WideToUtf8(const std::wstring& wide);


BOOL selectFile(std::wstring& selectedFile);