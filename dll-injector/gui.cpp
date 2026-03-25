#include <windows.h>
#include <cstdint>
#include <algorithm>
#include <commdlg.h>

#pragma comment(lib, "comdlg32.lib")

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "logger.h"
#include "procenum.h"
#include "inject.h"
#include "handles.h"
#include "utils.h"

void RenderUI(HWND hwnd) {

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    static std::vector<ProcessInfo> processes;
    static DWORD g_selectedPid = 0;
    static char g_searchBuf[128] = "";
    static bool g_loaded = false;
    static bool g_clicked = false;
    static bool g_showlog = false;
    static std::wstring g_dllFile = L"";

    // Main window

    ImGui::Begin("DLL Injector", nullptr, window_flags);

    if (ImGui::Button("Refresh") || !g_loaded) {
        processes = GetRunningProcesses();
        g_selectedPid = 0;
        g_loaded = true;
        g_clicked = false;
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputText("Search", g_searchBuf, sizeof(g_searchBuf));

    ImGui::SameLine();
    if (ImGui::Button(g_showlog ? "Hide Log" : "Show Log"))
        g_showlog = !g_showlog;

    // X button, I don't want the win32 window bar
    float buttonWidth = 25.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float rightEdge = ImGui::GetWindowWidth() - (buttonWidth * 2);

    ImGui::SameLine();
    ImGui::SetCursorPosX(rightEdge);

    if (ImGui::Button("X", ImVec2(buttonWidth, 0)))
        ::PostQuitMessage(0);

    ImGui::Separator();

    // Process table

    if (ImGui::BeginTable("ProcessTable", 2,
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_SizingFixedFit,
        ImVec2(0, 600)))
    {
        ImGui::TableSetupScrollFreeze(0, 1); // Keep header visible when scrolling
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (int i = 0; i < (int)processes.size(); i++) {
            const auto& proc = processes[i];

            // Search bar function
            if (g_searchBuf[0] != '\0') {
                std::string haystack = proc.name;
                std::string needle = g_searchBuf;
                std::transform(haystack.begin(), haystack.end(), haystack.begin(), ::tolower);
                std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
                if (haystack.find(needle) == std::string::npos)
                    continue;
            }

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            char pidStr[16];
            snprintf(pidStr, sizeof(pidStr), "%u", proc.pid);
            bool selected = (g_selectedPid == proc.pid);
            if (ImGui::Selectable(pidStr, selected,
                ImGuiSelectableFlags_SpanAllColumns,
                ImVec2(0, 0)))
            {
                g_selectedPid = proc.pid;
                Log(LogLevel::Info, "Selected PID: " + std::to_string(g_selectedPid) + " " + proc.name);
            
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", proc.name.c_str());
        }
        ImGui::EndTable();
    }

    
    // Select a process and inject it //

    // Select by PID
    auto it = std::find_if(processes.begin(), processes.end(),
        [](const ProcessInfo& p) { return p.pid == g_selectedPid; });

    if (it != processes.end()) {
        const auto& sel = *it;

		ImGui::Text("Selected [%u] %s", sel.pid, sel.name.c_str());

        // Inject / file browse window
        if (ImGui::Button("Inject")) {
            g_clicked = true;
        }

        if (g_clicked) {
            ImGui::Separator();
            ImGui::Spacing();

            // Child to hold the file browser "browse" button, and "execute" button
            ImGui::BeginChild("Inject Panel", ImVec2(0, 100), ImGuiChildFlags_Borders);

            ImGui::Text("Select DLL to Inject");
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Browse"))
                selectFile(g_dllFile);

            ImGui::SameLine();

            if (g_dllFile.empty())
                ImGui::TextDisabled("No DLL selected...");
            else
                ImGui::Text("%s", WideToUtf8(g_dllFile).c_str());

            ImGui::Spacing();

            if (g_dllFile.empty()) {
                ImGui::BeginDisabled();
                ImGui::Button("Execute");
                ImGui::EndDisabled();
            }
            else {
                if (ImGui::Button("Execute")) {

                    Log(LogLevel::Info, "Attempting to inject into PID: " + std::to_string(sel.pid) + " " + sel.name);
                    HANDLE hProcess = nullptr;
                    
                    if (!GetRemoteProcessHandle(sel.pid, &hProcess)) {
                        return;
                    }

                    if (!InjectDllToRemoteProcess(hProcess, (LPWSTR)g_dllFile.c_str())) {
                        return;
                    }
                }
            }
            ImGui::EndChild();
        }
	}
	else {
		ImGui::TextDisabled("No process selected");
	}


    // New window for the logger, opens with g_showlog

    if (g_showlog) {
        ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(20, 500), ImGuiCond_FirstUseEver);

        ImGui::Begin("Log", &g_showlog);

        if (ImGui::Button("Clear"))
            ClearLog();

        ImGui::SameLine();
        ImGui::TextDisabled("%d entries", (int)GetLog().size());

        ImGui::Separator();

        ImGui::BeginChild("LogEntries", ImVec2(0, 0), ImGuiChildFlags_None);

        for (const auto& msg : GetLog()) {
            if (msg.find("[+]") == 0)
                ImGui::TextColored(ImVec4(0.31f, 0.98f, 0.48f, 1.0f), "%s", msg.c_str());
            else if (msg.find("[!]") == 0)
                ImGui::TextColored(ImVec4(1.00f, 0.33f, 0.33f, 1.0f), "%s", msg.c_str());
            else if (msg.find("[~]") == 0)
                ImGui::TextColored(ImVec4(1.00f, 0.72f, 0.42f, 1.0f), "%s", msg.c_str());
            else
                ImGui::Text("%s", msg.c_str());
        }
        // auto scroll to bottom
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }

	ImGui::End();
}


