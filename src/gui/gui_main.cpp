#include "pch.hpp"
#include "Utils/Utils.hpp"
#include "GUI Utils.hpp"
#include "resource.hpp"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")

DWORD WINAPI MainThread(HWND);

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hProgress;
HWND hLabelStatic;
HWND hProgressStatic;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {

	if (getOsVersion() < 16299) {
		MessageBox(NULL, L"Your OS Version isn't supported.\nMinimal OS version 10.0.16299", L"Error", MB_ICONERROR | MB_OK);
		return 1;
	}

	WNDCLASSEX wClass;
	ZeroMemory(&wClass, sizeof(WNDCLASSEX));

	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.hbrBackground = (HBRUSH)COLOR_WINDOW; 
	wClass.hInstance = hInstance; 
	wClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wClass.lpfnWndProc = (WNDPROC)WndProc; 
	wClass.lpszClassName = L"Main Window Class"; 

	if (!RegisterClassEx(&wClass)) {
		gui::ErrorMessageBox();
		return -1;
	}

	const int width = 300;
	const int height = 150;

	int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	HWND hWindow = CreateWindowEx(NULL, L"Main Window Class", L"FS Setup", 
		WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		x, y, width, height, NULL, NULL, hInstance, NULL);

	if (!hWindow) {
		gui::ErrorMessageBox();
		return -1;
	}

	ShowWindow(hWindow, nShowCmd);

	MSG msg;

	ZeroMemory(&msg, sizeof(MSG));

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
	case WM_CREATE: {

		HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

		hProgress = CreateWindowEx(NULL, PROGRESS_CLASS, NULL,
			WS_CHILD | WS_VISIBLE | PBS_MARQUEE,
			20, 70, 245, 20, hwnd, NULL, hInst, NULL);

		HFONT hFont = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, UNICODE,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));

		hLabelStatic = CreateWindowEx(NULL, L"STATIC", L"Initializing...",
			WS_CHILD | WS_VISIBLE | SS_CENTER,
			20, 10, 245, 20, hwnd, NULL, hInst, NULL);
		SendMessage(hLabelStatic, WM_SETFONT, (WPARAM)hFont, true);

		hProgressStatic = CreateWindowEx(NULL, L"STATIC", NULL,
			WS_CHILD | WS_VISIBLE | SS_CENTER,
			20, 40, 245, 20, hwnd, NULL, hInst, NULL);
		SendMessage(hProgressStatic, WM_SETFONT, (WPARAM)hFont, true);

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, hwnd, NULL, NULL);
		break;
	};
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND gui::GetProgressHWND() {
	return hProgress;
}
HWND gui::GetLabelStaticHWND() {
	return hLabelStatic;
}
HWND gui::GetProgressStaticHWND() {
	return hProgressStatic;
}