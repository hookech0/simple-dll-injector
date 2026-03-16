#include <windows.h>

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

	ImGui::Begin("DLL 'jector", nullptr, window_flags);

	//static char dllPath[256] = "C:\\path\\to\\dll";
	//ImGui::InputText("DLL Path", dllPath, IM_ARRAYSIZE(dllPath));

	ImGui::BeginChild("Process List", ImVec2(0, 300), true);

	ImGuiListClipper clipper;


	if (ImGui::Button("Inject DLL")) {
		// insert proc enum and injector logic


	
	}

	ImGui::End();

}

