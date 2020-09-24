#pragma once
#include "pch.h"

#pragma warning(disable: 4244)

namespace gui {
	HWND GetProgressHWND();
	HWND GetLabelStaticHWND();
	HWND GetProgressStaticHWND();

	int ErrorMessageBox();
	void SetPending(bool value);

	void SetLabel(std::string text);
	void SetProgressStatic(std::string progress);
	void SetProgress(uint64_t, uint64_t);
	void SetFullProgress();
}