#include <windows.h>
#include <d3d11.h>
#include <tchar.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "gui.h"

// -- Mostly boilerplate from Imgui Win32/DX11 example code -- //

static ID3D11Device*			g_pd3dDevice = nullptr;
static ID3D11DeviceContext*		g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*			g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*	g_mainRenderTargetView = nullptr;
bool g_AppRunning = true;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void RenderUI();

int main(int, char**) {

	//ImGui_ImplWin32_EnableDpiAwareness();

	WNDCLASSEXW wc = {

		sizeof(wc),
		CS_CLASSDC,
		WndProc,
		0L,
		0L,
		GetModuleHandle(nullptr),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		L"DLL 'jector",
		nullptr
	};

	::RegisterClassExW(&wc);

	//HWND hwnd = ::CreateWindowW(
	//	wc.lpszClassName,
	//	L"DLL 'jector",
	//	WS_OVERLAPPEDWINDOW,
	//	100,
	//	100,
	//	1280, // Width
	//	800,  // Height
	//	nullptr,
	//	nullptr,
	//	wc.hInstance,
	//	nullptr
	//);

	HWND hwnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		wc.lpszClassName,
		L"DLL Injector",
		WS_POPUP | WS_VISIBLE,
		100, 100, 1280, 800,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	if (!CreateDeviceD3D(hwnd)) {
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	//::ShowWindow(hwnd, SW_SHOWDEFAULT);  
	//::ShowWindow(hwnd, SW_HIDE);
	::UpdateWindow(hwnd);


	// imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	io.Fonts->AddFontFromFileTTF("external/fonts/JetBrainsMono-Italic.ttf"); // external fonts

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	// ImGui custom style

	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowMinSize = ImVec2(160, 20);
	style->FramePadding = ImVec2(4, 2);
	style->ItemSpacing = ImVec2(6, 2);
	style->ItemInnerSpacing = ImVec2(6, 4);
	style->Alpha = 0.95f;
	style->WindowRounding = 4.0f;
	style->FrameRounding = 2.0f;
	style->IndentSpacing = 6.0f;
	style->ItemInnerSpacing = ImVec2(2, 4);
	style->ColumnsMinSpacing = 50.0f;
	style->GrabMinSize = 14.0f;
	style->GrabRounding = 16.0f;
	style->ScrollbarSize = 12.0f;
	style->ScrollbarRounding = 16.0f;

	// Dracula theme, created by claude

	style->Colors[ImGuiCol_Text] = ImVec4(0.97f, 0.97f, 0.95f, 1.00f); // #f8f8f2
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f); // #6272a4
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f); // #282a36
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f); // #282a36
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f); // #282a36

	// Borders — subtle purple tint
	style->Colors[ImGuiCol_Border] = ImVec4(0.74f, 0.58f, 0.98f, 0.40f); // #bd93f9 dimmed
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// FrameBg is what makes your search box visible
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f); // #44475a
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.74f, 0.58f, 0.98f, 0.30f); // purple tint
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.74f, 0.58f, 0.98f, 0.50f); // purple tint

	// Title
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f); // #282a36
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f); // #44475a
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.21f, 0.75f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f); // #44475a

	// Scrollbar
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.74f, 0.58f, 0.98f, 0.50f); // #bd93f9
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.74f, 0.58f, 0.98f, 0.75f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.74f, 0.58f, 0.98f, 1.00f);

	// Interactive accents — purple/pink
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.74f, 0.58f, 0.98f, 1.00f); // #bd93f9
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.74f, 0.58f, 0.98f, 0.80f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.47f, 0.78f, 1.00f); // #ff79c6

	// Buttons
	style->Colors[ImGuiCol_Button] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f); // #44475a
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.74f, 0.58f, 0.98f, 0.50f); // purple
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.47f, 0.78f, 0.80f); // pink

	// Table / Selectable headers
	style->Colors[ImGuiCol_Header] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f); // #44475a
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.74f, 0.58f, 0.98f, 0.40f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.74f, 0.58f, 0.98f, 0.60f);

	// Resize
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.74f, 0.58f, 0.98f, 0.20f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.74f, 0.58f, 0.98f, 0.60f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.47f, 0.78f, 1.00f);

	// Plots — green/orange accent
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.31f, 0.98f, 0.48f, 1.00f); // #50fa7b
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.72f, 0.42f, 1.00f); // #ffb86c
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.31f, 0.98f, 0.48f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.72f, 0.42f, 1.00f);

	// Selection
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.74f, 0.58f, 0.98f, 0.35f);
	style->Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);
	style->Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.74f, 0.58f, 0.98f, 0.30f);
	style->Colors[ImGuiCol_TableBorderLight] = ImVec4(0.74f, 0.58f, 0.98f, 0.15f);

	ImVec4 clear_color = ImVec4(0.16f, 0.16f, 0.21f, 1.00f); // matches WindowBg

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	
	while (g_AppRunning && msg.message != WM_QUIT) {
		if (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		RenderUI();

		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
		
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		g_pSwapChain->Present(1, 0);
		g_pd3dDeviceContext->Flush();		
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;


	switch (msg) {
	case WM_SIZE:
		if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	// This is a basic setup. Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. See #8979 for suggestions.
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}
