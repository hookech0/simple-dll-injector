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

void RenderUI() {


    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    extern ImFont* g_fontJetBrains;
    extern ImFont* g_fontCascadiaCode;
    extern ImFont* g_fontProggy;

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    //window_flags |= ImGuiWindowFlags_NoMove;
    //window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    static std::vector<ProcessInfo> processes;
    static DWORD g_selectedPid = 0;
    static char g_searchBuf[128] = "";
    static bool g_loaded = false;
    static bool g_clicked = false;
    static bool g_showlog = false;
    static std::wstring g_dllPath = L"";
   

    // Main window

    ImGui::Begin("DLL Injector", nullptr, window_flags);

    if (ImGui::BeginTabBar("InjectorTabBar")) {
        if (ImGui::BeginTabItem("Processes")) {

            if (ImGui::Button("Refresh") || !g_loaded) {
                processes = GetRunningProcesses();
                g_selectedPid = 0;
                g_loaded = true;
                g_clicked = false;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Refresh process list"); // Not sure if I really need a tooltip for this 
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(230.0f);
            ImGui::InputText("\t", g_searchBuf, sizeof(g_searchBuf)); // \t hides the label all together. I would prefer the label to be "inside" the text input field
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Search for a process name");
            }

            ImGui::Separator();

            // ---- Process Table ---- //

            ImGuiTableFlags table_flags = 0;
            table_flags |= ImGuiTableFlags_Borders;
            table_flags |= ImGuiTableFlags_RowBg;
            table_flags |= ImGuiTableFlags_ScrollY;
            table_flags |= ImGuiTableFlags_SizingFixedFit;


            if (ImGui::BeginTable("ProcessTable", 2, table_flags, ImVec2(0, 600))) {

                ImGui::TableSetupScrollFreeze(0, 1); // Keep table header visible when scrolling
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

            // --------------------------------------- //

            // ---- Process Selection & Injection ---- //

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

                // If the inject button is clicked, open file browsing child window
                if (g_clicked) {
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Child to hold the file browser "browse" button, and "execute" button
                    ImGui::BeginChild("Inject Panel", ImVec2(0, 100), ImGuiChildFlags_Borders);

                    ImGui::Text("Select DLL to Inject");
                    ImGui::Separator();
                    ImGui::Spacing();

                    if (ImGui::Button("Browse"))
                        selectFile(g_dllPath);

                    ImGui::SameLine();

                    if (g_dllPath.empty())
                        ImGui::TextDisabled("No DLL selected...");
                    else
                        ImGui::Text("%s", WideToUtf8(g_dllPath).c_str());

                    ImGui::Spacing();

                    if (g_dllPath.empty()) {
                        ImGui::BeginDisabled();
                        ImGui::Button("Execute");
                        ImGui::EndDisabled();
                    }
                    else {
                        // --- Process Injection --- //
                        if (ImGui::Button("Execute")) {

                            Log(LogLevel::Info, "Attempting to inject into PID: " + std::to_string(sel.pid) + " " + sel.name);
                            HANDLE hProcess = nullptr;

                            if (!GetRemoteProcessHandle(sel.pid, &hProcess)) {
                                return;
                            }

                            if (!InjectDllToRemoteProcess(hProcess, (LPWSTR)g_dllPath.c_str())) {
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
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Log")) {
            
            if (ImGui::Button("Clear Log")) {
                ClearLog();
            }

            ImGui::Separator();

            ImGui::PushFont(g_fontProggy);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.07f, 0.027f, 0.027f, 1.0f));

            ImGui::BeginChild("Logger", ImVec2(0, 0), ImGuiChildFlags_None);

            for (const auto& msg : GetLog()) {
                if (msg.find("[+]") == 0)
                    ImGui::TextColored(ImVec4(0.31f, 0.98f, 0.48f, 1.0f), "%s", msg.c_str()); //  Green
                else if (msg.find("[!]") == 0)
                    ImGui::TextColored(ImVec4(1.00f, 0.33f, 0.33f, 1.0f), "%s", msg.c_str()); // Red
                else if (msg.find("[~]") == 0)
                    ImGui::TextColored(ImVec4(1.00f, 0.72f, 0.42f, 1.0f), "%s", msg.c_str()); // Orangish
                else
                    ImGui::Text("%s", msg.c_str()); // No colour
            }
            // Auto scroll to bottom
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            
            ImGui::EndChild();
            ImGui::PopFont();
            ImGui::PopStyleColor();

            ImGui::EndTabItem();
        }
 

    }
    ImGui::EndTabBar();



    // --------------------------------------- //


    // --- Logger Window --- //

    // New window for the logger, opens with g_showlog
    // Maybe move to a tab instead of a new window


//    if (g_showlog) {
//        ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);
//        ImGui::SetNextWindowPos(ImVec2(20, 500), ImGuiCond_FirstUseEver);
//
//        ImGui::Begin("Log", &g_showlog);
//
//        if (ImGui::Button("Clear"))
//            ClearLog();
//
//        ImGui::SameLine();
//        ImGui::TextDisabled("%d entries", (int)GetLog().size());
//
//        ImGui::Separator();
//
//        ImGui::BeginChild("Logger", ImVec2(0, 0), ImGuiChildFlags_None);
//
//        for (const auto& msg : GetLog()) {
//            if (msg.find("[+]") == 0)
//                ImGui::TextColored(ImVec4(0.31f, 0.98f, 0.48f, 1.0f), "%s", msg.c_str()); //  Green
//            else if (msg.find("[!]") == 0)
//                ImGui::TextColored(ImVec4(1.00f, 0.33f, 0.33f, 1.0f), "%s", msg.c_str()); // Red
//            else if (msg.find("[~]") == 0)
//                ImGui::TextColored(ImVec4(1.00f, 0.72f, 0.42f, 1.0f), "%s", msg.c_str()); // Orangish
//            else
//                ImGui::Text("%s", msg.c_str()); // No colour
//        }
//        // auto scroll to bottom
//        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
//            ImGui::SetScrollHereY(1.0f);
//
//        ImGui::EndChild();
//        ImGui::End();
//    }
//
// 
// 

// --------------------------------------- //

	ImGui::End();
}