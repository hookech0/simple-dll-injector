#include "logger.h"

static std::vector<std::string> g_log;

void Log(LogLevel level, const std::string& message) {
	std::string prefix;


	switch (level) {
		case LogLevel::Info: prefix = "[i] "; break;
		case LogLevel::Success: prefix = "[+] "; break;
		case LogLevel::Warning: prefix = "[~] "; break;
		case LogLevel::Error: prefix = "[!] "; break;
	}

	g_log.push_back(prefix + message);
}

const std::vector<std::string>& GetLog() {
	return g_log;
}

void ClearLog() {
	g_log.clear();
}