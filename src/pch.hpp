#pragma once

// Pre-compiled headers

// STD stuff
#include <filesystem>
#include <optional>
#include <fstream>
#include <thread>

// Windows
#include <windows.h>
#include <CommCtrl.h>
#include <appmodel.h>

#include <winrt/Windows.Management.Deployment.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Web.Http.Headers.h>

// Additional dependencies
#include <boost/algorithm/hex.hpp>
#include <fmt/format.h>
#include <pugixml.hpp>

class Console {

public:

	Console() {

		AllocConsole();
		freopen_s(&f, "CONOUT$", "w", stdout);
	}
	~Console() {

		system("pause");
		fclose(f);
		FreeConsole();
	}

private:
	FILE* f;
};