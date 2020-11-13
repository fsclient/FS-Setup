#include "pch.h"
#include "Utils.h"

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

bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string thumbPrint) {

	PCCERT_CONTEXT pCertContext = NULL;

	while (pCertContext = CertEnumCertificatesInStore(hRootCertStore, pCertContext)) {

		DWORD length = 0;

		CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, nullptr, &length);

		auto buffer = std::make_shared<BYTE[]>(length);
		if (buffer == nullptr) return false;

		CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, buffer.get(), &length);

		if (ByteArrayToString(buffer) == thumbPrint) return true;
	}
	return false;
}

std::string ByteArrayToString(std::shared_ptr<BYTE[]> BytePtr) {

	std::string hexString;

	boost::algorithm::hex(BytePtr.get(), std::back_inserter(hexString));
	boost::algorithm::to_lower(hexString);

	return hexString;
}

std::string httpGet(std::string_view url) {

	std::string result;

	winrt::Windows::Web::Http::HttpClient client;
	winrt::Windows::Foundation::Uri uri(winrt::to_hstring(url));

	auto response = client.GetAsync(uri).get();

	result.reserve(response.Content().Headers().ContentLength().GetUInt64());
	result = winrt::to_string(response.Content().ReadAsStringAsync().get());

	return result;
}

std::filesystem::path createFile(std::filesystem::path fp, std::string data) {

	std::ofstream file;
	file.open(fp, std::ios::binary | std::ios::out);
	file << data;
	file.close();

	return fp;
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

DWORD getOsVersion() {

	NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);
	OSVERSIONINFOEXW osInfo;

	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

	if (NULL != RtlGetVersion) {

		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);

		return osInfo.dwBuildNumber;
	}

	return 0;
}
