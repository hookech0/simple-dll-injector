#include <windows.h>
#include <cstdint>
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "procenum.h"
#include "inject.h"





void RenderUI() {

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f)); // Place window in the top left
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize); // Make window size of the win32 window

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    static std::vector<ProcessInfo> processes;
    static int selectedProcess = -1;
    static char searchBuf[128] = "";
    static bool loaded = false;

    ImGui::Begin("DLL Injector", nullptr, window_flags);

    if (ImGui::Button("Refresh") || !loaded) {
        processes = GetRunningProcesses();
        selectedProcess = -1;
        loaded = true;
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputText("Search", searchBuf, sizeof(searchBuf));

    ImGui::Separator();

    if (ImGui::BeginTable("ProcessTable", 2,
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_SizingFixedFit,
        ImVec2(0, 600))) // table size
    {
        ImGui::TableSetupScrollFreeze(0, 1); // Keep header visible when scrolling
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (int i = 0; i < (int)processes.size(); i++) {
            const auto& proc = processes[i];

            // Search filter
            if (searchBuf[0] != '\0') {
                std::string haystack = proc.name;
                std::string needle = searchBuf;
                std::transform(haystack.begin(), haystack.end(), haystack.begin(), ::tolower);
                std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
                if (haystack.find(needle) == std::string::npos)
                    continue;
            }

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            char pidStr[16];
            snprintf(pidStr, sizeof(pidStr), "%u", proc.pid);
            bool selected = (selectedProcess == i);
            if (ImGui::Selectable(pidStr, selected,
                ImGuiSelectableFlags_SpanAllColumns,
                ImVec2(0, 0)))
            {
                selectedProcess = i;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", proc.name.c_str());
        }

        ImGui::EndTable();
    }

	if (selectedProcess >= 0 && selectedProcess < (int)processes.size()) {
		const auto& sel = processes[selectedProcess];
		ImGui::Text("Selected [%u] %s", sel.pid, sel.name.c_str());

		if (ImGui::Button("Inject")) {
			// injector logic here
		}
	}
	else {
		ImGui::TextDisabled("No process selected");
	}
	ImGui::End();
}

// Selectable -- not in a table

//	ImGui::BeginChild("Process List", ImVec2(0, 300), true);
//
//	for (int i = 0; i < (int)processes.size(); i++) {
//		const auto& proc = processes[i];
//
//		if (searchBuf[0] != '\0') {
//			std::string pname = proc.name;
//			std::string psearch = searchBuf;
//			std::transform(pname.begin(), pname.end(), pname.begin(), ::tolower);
//			std::transform(psearch.begin(), psearch.end(), psearch.begin(), ::tolower);
//			if (pname.find(psearch) == std::string::npos) {
//				continue;
//			}
//		}
//
//		char label[256];
//		snprintf(label, sizeof(label), "%-8u %s", proc.pid, proc.name.c_str());
//
//		bool selected = (selectedProcess == i);
//		if (ImGui::Selectable(label, selected))
//			selectedProcess = i;
//	}
//	ImGui::EndChild();
//
//	
//	if (selectedProcess >= 0 && selectedProcess < (int)processes.size()) {
//		const auto& sel = processes[selectedProcess];
//		ImGui::Text("Selected [%u] %s", sel.pid, sel.name.c_str());
//
//		if (ImGui::Button("Inject")) {
//			// injector logic here
//		}
//	}
//	else {
//		ImGui::TextDisabled("No process selected");
//	}
//
//	ImGui::End();
//
//}

