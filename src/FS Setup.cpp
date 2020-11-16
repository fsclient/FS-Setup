#include "pch.hpp"
#include "gui/GUI Utils.hpp"
#include "Utils/Utils.hpp"

void install_certificate();
void manualInstall();

DWORD WINAPI MainThread(HWND MainWindow) {

	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	int minOsVer = 16299;

	if (getOsVersion() >= minOsVer) {

		install_certificate();

		if (isPackageExists("Microsoft.DesktopAppInstaller")) {
			ShellExecute(NULL, L"open", L"ms-appinstaller:?source=https://fsclient.github.io/fs/FSClient.UWP.appinstaller", NULL, NULL, SW_SHOWDEFAULT);
		}	
		else {
			
			manualInstall();

			gui::SetPending(false);
			gui::SetFullProgress();;

			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
	}
	else MessageBox(MainWindow, L"Your OS Version isn't supported.", L"Error", MB_ICONERROR | MB_OK);
	
	SendMessage(MainWindow, WM_DESTROY, 0, 0);
	return 0;

	fclose(f);
	FreeConsole();
}

void manualInstall() {

	gui::SetLabel("Trying to manual install...");

	std::string body = httpGet("https://fsclient.github.io/fs/FSClient.UWP.appinstaller");

	pugi::xml_document doc;
	doc.load_string(body.c_str());

	if (auto package = getPackageByFamilyName("24831TIRRSOFT.FS_7dqv9t6ww56qc")) {

		std::string_view version = doc.child("AppInstaller").attribute("Version").value();

		const auto& [major, minor, build, revision] = package->Id().Version();
		std::string localVersion = fmt::format("{}.{}.{}.{}", major, minor, build, revision);

		using winrt::Windows::System::ProcessorArchitecture;

		ProcessorArchitecture pkgArchitecture = package->Id().Architecture();

		if (localVersion == version && pkgArchitecture == ProcessorArchitecture::X64) {
			gui::SetLabel("FS Client is up-to-dated");
			return;
		}
	}

	// #TODO Make it works
	winrt::Windows::Management::Deployment::PackageManager manager;
	std::filesystem::path tempDir = std::filesystem::temp_directory_path();

	for (pugi::xml_node package : doc.child("AppInstaller").child("Dependencies").children("Package")) {

		if (std::strcmp(package.attribute("ProcessorArchitecture").value(), "x64"))
			continue;

		std::string_view pkgName = package.attribute("Name").value();

		if (!isPackageExists(pkgName)) {

			gui::SetLabel(pkgName);

			std::string body = httpGet(package.attribute("Uri").value());

			std::filesystem::path tempFilePath = tempDir.string() + pkgName.data() + ".appx";
			std::ofstream file(tempFilePath, std::ios::binary);
			file << body; file.close();

			winrt::Windows::Foundation::Uri uri(L"file://" + tempFilePath.wstring());

			auto deploymentOperation = manager.AddPackageAsync(uri, NULL,
				winrt::Windows::Management::Deployment::DeploymentOptions::None);
			deploymentOperation.get();

			if (deploymentOperation.Status() == winrt::Windows::Foundation::AsyncStatus::Error) {

				fmt::print("Url: {}\n", package.attribute("Uri").value());
				fmt::print("Local Uri: {}\n", winrt::to_string(uri.ToString()));
				fmt::print("Error Code: {}\n", deploymentOperation.ErrorCode().value);
				fmt::print("Error message: {}\n", winrt::to_string(deploymentOperation.GetResults().ErrorText()));

				system("pause");
			}

			std::filesystem::remove(tempFilePath);
		}
	}

// 	body = httpGet("https://fsclient.github.io/fs/FSClient.UWP/FSClient.UWP.appxbundle");
}

void install_certificate() {

	gui::SetLabel("Installing certificate...");
	gui::SetPending(true);

	HCERTSTORE hRootCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING, NULL, CERT_SYSTEM_STORE_LOCAL_MACHINE, L"Root");

	if (!CheckCertByThumbPrint(hRootCertStore, "24542010cb06ad6ab320c84a984c7862e029fb08")) {

		std::string body = httpGet("https://fsclient.github.io/fs/FSClient.UWP/FSClient.UWP.cer");

		PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING, (const BYTE*)body.c_str(), (DWORD)body.size() + 1);

		if (pCertContext != NULL) {
			CertAddCertificateContextToStore(hRootCertStore, pCertContext, CERT_STORE_ADD_USE_EXISTING, NULL);
			CertFreeCertificateContext(pCertContext);
		}	
	}
	CertCloseStore(hRootCertStore, 0);
}