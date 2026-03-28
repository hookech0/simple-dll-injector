#pragma once

#include <vector>
#include <string>

enum class LogLevel {

	Info,
	Success,
	Warning,
	Error

};

void Log(LogLevel level, const std::string& message);

const std::vector<std::string>& GetLog();

void ClearLog();