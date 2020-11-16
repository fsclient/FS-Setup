#include "pch.hpp"
#include "Utils.hpp"
#include "gui/GUI Utils.hpp"

#pragma warning(disable : 6387)

bool isPackageExists(std::string_view pkgName) {

	auto packageName = winrt::to_hstring(pkgName);

	winrt::Windows::Management::Deployment::PackageManager manager;
	auto collection = manager.FindPackagesForUser(winrt::hstring());
	auto packages = collection.First();

	do {
		auto package = packages.Current();
		if (package.Id().Name() == packageName)
			return true;

	} while (packages.MoveNext());

	return false;
}

std::optional<winrt::Windows::ApplicationModel::Package> getPackageByFamilyName(std::string_view familyName) {

	auto hFamilyName = winrt::to_hstring(familyName);

	winrt::Windows::Management::Deployment::PackageManager manager;
	auto collection = manager.FindPackagesForUser(winrt::hstring());
	auto packages = collection.First();

	do {
		auto package = packages.Current();
		if (package.Id().FamilyName() == hFamilyName)
			return package;

	} while (packages.MoveNext());

	return std::nullopt;
}

bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string_view thumbPrint) {

	PCCERT_CONTEXT pCertContext = NULL;

	while (pCertContext = CertEnumCertificatesInStore(hRootCertStore, pCertContext)) {

		DWORD length = 0;

		CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, nullptr, &length);

		auto buffer = std::make_unique<BYTE[]>(length);
		if (buffer == nullptr) return false;

		CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, buffer.get(), &length);

		std::string hexString;
		boost::algorithm::hex_lower(buffer.get(), std::back_inserter(hexString));

		if (hexString == thumbPrint) return true;
	}
	return false;
}

std::string httpGet(std::string_view url) {

	const enum sizes : unsigned int {
		Byte = 1,
		kiloByte = Byte * 1024,
		megaByte = kiloByte * 1024,
	};

	std::string result;
	winrt::Windows::Web::Http::HttpClient client;

	auto asyncOperation = client.GetAsync(winrt::Windows::Foundation::Uri(winrt::to_hstring(url)));
	asyncOperation.Progress([=](auto ResponseMessage, winrt::Windows::Web::Http::HttpProgress progressInfo) {
		
		if (progressInfo.TotalBytesToReceive) {

			float recieved = static_cast<float>(progressInfo.BytesReceived);
			float total = static_cast<float>(progressInfo.TotalBytesToReceive.Value());

			std::string outString;

			if (total >= sizes::megaByte) fmt::format_to(std::back_inserter(outString), "Recieved {0:.3f}/{1:.3f} MiB", recieved / sizes::megaByte, total / sizes::megaByte);
			else if (total >= sizes::kiloByte) fmt::format_to(std::back_inserter(outString), "Recieved {0:.3f}/{1:.3f} KiB", recieved / sizes::kiloByte, total / sizes::kiloByte);
			else fmt::format_to(std::back_inserter(outString), "Recieved {}/{} bytes", recieved, total);

			gui::SetProgressStatic(outString);
			if (total >= sizes::megaByte * 5) {
				gui::SetPending(false);
				gui::SetProgress(progressInfo.BytesReceived, progressInfo.TotalBytesToReceive.Value());
			}
		}
	});

	auto response = asyncOperation.get();
	result.reserve(response.Content().Headers().ContentLength().Value());
	result = winrt::to_string(response.Content().ReadAsStringAsync().get());

	
	gui::SetFullProgress();
	gui::SetProgressStatic("Download Complete.");

	std::this_thread::sleep_for(std::chrono::milliseconds(700));
	gui::SetPending(true);

	return result;
}

int getOsVersion(void) {

	NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);
	OSVERSIONINFOEXW osInfo;

	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

	if (RtlGetVersion != NULL) {

		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);

		return static_cast<int>(osInfo.dwBuildNumber);
	}

	return 0;
}

bool installPackageByPath(std::filesystem::path filePath) {

	//PowerShell.exe -Command "Add-AppxPackage 'path' -AppInstallerFile"
	std::string command = fmt::format("-Command \"Add-AppxPackage '{}'\"", filePath.string());

	if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE) == S_OK) {

		SHELLEXECUTEINFO ShExecInfo;

		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = L"open";
		ShExecInfo.lpFile = L"PowerShell.exe";
		ShExecInfo.lpParameters = winrt::to_hstring(command).c_str();
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_SHOWDEFAULT;
		ShExecInfo.hInstApp = NULL;

		ShellExecuteEx(&ShExecInfo);
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
		CloseHandle(ShExecInfo.hProcess);

		return true;
	}
	return false;
}
