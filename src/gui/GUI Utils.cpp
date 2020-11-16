#include "pch.hpp"
#include "GUI Utils.hpp"

int gui::ErrorMessageBox() {
	std::wstring message = L"An error was occurred with code: ";
	std::wstring errorNum = std::to_wstring(GetLastError());
	return MessageBox(NULL, std::wstring(message + errorNum).c_str(), L"Error", MB_ICONERROR);
}

void gui::SetPending(bool value) {

	static bool enabled;

	HWND hProgress = GetProgressHWND();

	if (value && !enabled) {
		auto style = GetWindowLongPtr(hProgress, GWL_STYLE);
		style |= PBS_MARQUEE;
		SetWindowLongPtr(hProgress, GWL_STYLE, style);
		SendMessage(hProgress, PBM_SETMARQUEE, true, 0);
		enabled = true;
	}
	else if (!value && enabled) {
		auto style = GetWindowLongPtr(hProgress, GWL_STYLE);
		style &= ~PBS_MARQUEE;
		SetWindowLongPtr(hProgress, GWL_STYLE, style);
		SendMessage(hProgress, PBM_SETMARQUEE, false, 0);
		enabled = false;
	}
}

void gui::SetLabel(std::string_view text) {
	HWND hLabelStatic = GetLabelStaticHWND();
	SetWindowText(hLabelStatic, winrt::to_hstring(text).c_str());
}

void gui::SetProgressStatic(std::string_view progress) {
	HWND hProgressStatic = GetProgressStaticHWND();
	SetWindowText(hProgressStatic, winrt::to_hstring(progress).c_str());
}

void gui::SetProgress(uint64_t len, uint64_t total) {
	HWND hProgress = GetProgressHWND();

	SendMessage(hProgress, PBM_SETRANGE32, 0, total);
	SendMessage(hProgress, PBM_SETPOS, len, 0);
}

void gui::SetFullProgress() {
	HWND hProgress = GetProgressHWND();
	int maxValue = SendMessage(hProgress, PBM_GETRANGE, false, NULL);
	SendMessage(hProgress, PBM_SETPOS, maxValue, 0);
}