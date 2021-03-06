#include "pch.hpp"
#include "Utils.hpp"
#include "gui/GUI Utils.hpp"

#pragma warning(disable : 6387)

std::string httpGet(std::string_view url) {

	const enum sizes : unsigned int {
		Byte = 1,
		kiloByte = Byte * 1024,
		megaByte = kiloByte * 1024,
	};

	winrt::Windows::Web::Http::HttpClient client;

	auto asyncOperation = client.GetAsync(winrt::Windows::Foundation::Uri(winrt::to_hstring(url)));
	asyncOperation.Progress([](auto ResponseMessage, winrt::Windows::Web::Http::HttpProgress progressInfo) {

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
	winrt::hstring result = response.Content().ReadAsStringAsync().get();

	gui::SetFullProgress();
	gui::SetProgressStatic("Download Complete.");

	std::this_thread::sleep_for(std::chrono::milliseconds(700));
	gui::SetPending(true);

	return { std::begin(result), std::end(result) };
}

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

winrt::Windows::Foundation::AsyncStatus installPackageByAppInstallerUrl(std::string_view url) {

	winrt::Windows::Management::Deployment::PackageManager manager;
	std::filesystem::path tempDir = std::filesystem::temp_directory_path();

	std::string body = httpGet(url);

	std::filesystem::path tempFilePath = tempDir.string() + "package.appinstaller";
	std::ofstream file(tempFilePath, std::ios::binary);
	file << body; file.close();

	winrt::Windows::Foundation::Uri uri(L"file://" + tempFilePath.wstring());

	auto deploymentOperation = manager.AddPackageByAppInstallerFileAsync(uri, 
		winrt::Windows::Management::Deployment::AddPackageByAppInstallerOptions::None,
		manager.GetDefaultPackageVolume());

	deploymentOperation.Progress([](auto DeploymentResult, winrt::Windows::Management::Deployment::DeploymentProgress progressInfo) {
		gui::SetPending(false);
		gui::SetProgress(progressInfo.percentage, 100);
	});

	gui::SetProgressStatic("Installing...");
	deploymentOperation.get();

	return deploymentOperation.Status();
}

winrt::Windows::Foundation::AsyncStatus installPackageByUrl(std::string_view url) {

	winrt::Windows::Management::Deployment::PackageManager manager;
	std::filesystem::path tempDir = std::filesystem::temp_directory_path();

	std::string body = httpGet(url);

	std::filesystem::path tempFilePath = tempDir.string() + "package.appx";
	std::ofstream file(tempFilePath, std::ios::binary);
	file << body; file.close();

	winrt::Windows::Foundation::Uri uri(L"file://" + tempFilePath.wstring());

	auto deploymentOperation = manager.AddPackageAsync(uri, NULL,
		winrt::Windows::Management::Deployment::DeploymentOptions::None);

	deploymentOperation.Progress([](auto DeploymentResult, winrt::Windows::Management::Deployment::DeploymentProgress progressInfo) {
			gui::SetPending(false);
			gui::SetProgress(progressInfo.percentage, 100);
	});

	gui::SetProgressStatic("Installing...");
	deploymentOperation.get();

	std::filesystem::remove(tempFilePath);

	return deploymentOperation.Status();
}

bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string_view thumbPrint) {

	auto hex = [](std::byte* bytes, size_t length) {

		std::string result;
		std::string_view hexDigits = "0123456789abcdef";

		for (size_t i = 0; i < length; i++) {
			result.append(&hexDigits[((int)bytes[i] & 0xf0) >> 4], 1);
			result.append(&hexDigits[((int)bytes[i] & 0x0f) >> 0], 1);
		}

		return result;
	};

	PCCERT_CONTEXT pCertContext = NULL;

	while (pCertContext = CertEnumCertificatesInStore(hRootCertStore, pCertContext)) {

		DWORD length = 0;

		CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, nullptr, &length);

		auto buffer = std::make_unique<std::byte[]>(length);
		if (buffer == nullptr) return false;

		CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, buffer.get(), &length);

		if (hex(buffer.get(), length) == thumbPrint)
			return true;
	}
	return false;
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
