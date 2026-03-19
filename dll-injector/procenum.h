#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct ProcessInfo {
    uint32_t pid;
    std::string name;
};

std::vector<ProcessInfo> GetRunningProcesses();