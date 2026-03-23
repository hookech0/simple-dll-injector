#include <windows.h>
#include <cstdint>
#include <algorithm>
#include <commdlg.h>

#pragma comment(lib, "comdlg32.lib")

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "procenum.h"
#include "inject.h"
#include "handles.h"
#include "utils.h"

void RenderUI() {

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f)); // Place window in the top left
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize); // Make window size of the win32 window

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    // Static vars
    static std::vector<ProcessInfo> processes;
    static int g_selectedProcess = -1;
    static char g_searchBuf[128] = "";
    static bool g_loaded = false;
    static bool g_clicked = false;
    static std::wstring g_dllFile = L"";


    ImGui::Begin("DLL Injector", nullptr, window_flags);

    if (ImGui::Button("Refresh") || !g_loaded) {
        processes = GetRunningProcesses();
        g_selectedProcess = -1;
        g_loaded = true;
        g_clicked = false;
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputText("Search", g_searchBuf, sizeof(g_searchBuf));

    ImGui::Separator();

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
            bool selected = (g_selectedProcess == i);
            if (ImGui::Selectable(pidStr, selected,
                ImGuiSelectableFlags_SpanAllColumns,
                ImVec2(0, 0)))
            {
                g_selectedProcess = i;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", proc.name.c_str());
        }

        ImGui::EndTable();
    }

	if (g_selectedProcess >= 0 && g_selectedProcess < (int)processes.size()) {
		const auto& sel = processes[g_selectedProcess];
		ImGui::Text("Selected [%u] %s", sel.pid, sel.name.c_str());


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
                    
                    HANDLE hProcess = nullptr;
                    
                    // Gets a handle to the selected process
                    if (!GetRemoteProcessHandle(sel.pid, &hProcess)) {
                        return;
                    }

                    // Trys to inject the selected dll into the selected process
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
	ImGui::End();
}