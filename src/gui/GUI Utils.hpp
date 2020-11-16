#pragma once
#include "pch.hpp"

#pragma warning(disable: 4244)

namespace gui {
	HWND GetProgressHWND();
	HWND GetLabelStaticHWND();
	HWND GetProgressStaticHWND();

	int ErrorMessageBox();
	void SetPending(bool value);

	void SetLabel(std::string_view text);
	void SetProgressStatic(std::string_view progress);
	void SetProgress(uint64_t, uint64_t);
	void SetFullProgress();
}