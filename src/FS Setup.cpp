#include "pch.hpp"
#include "gui/GUI Utils.hpp"
#include "Utils/Utils.hpp"

bool manualInstall();

DWORD WINAPI MainThread(HWND MainWindow) {

	{
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

	if (isPackageExists("Microsoft.DesktopAppInstaller")) {

		gui::SetLabel("Trying to auto install...");

		auto status = installPackageByAppInstallerUrl("https://fsclient.github.io/fs/FSClient.UWP.appinstaller");

		if (status == winrt::Windows::Foundation::AsyncStatus::Completed)
			gui::SetProgressStatic("Successfully installed.");
		else if (status == winrt::Windows::Foundation::AsyncStatus::Error)
			manualInstall();

		//ShellExecute(NULL, L"open", L"ms-appinstaller:?source=https://fsclient.github.io/fs/FSClient.UWP.appinstaller", NULL, NULL, SW_SHOWDEFAULT);
	}	
	else manualInstall();

	gui::SetPending(false);
	gui::SetFullProgress();

	std::this_thread::sleep_for(std::chrono::seconds(3));
	
	SendMessage(MainWindow, WM_DESTROY, 0, 0);
	return 0;
}

bool manualInstall() {

	gui::SetLabel("Trying to manual install...");

	std::string body = httpGet("https://fsclient.github.io/fs/FSClient.UWP.appinstaller");

	pugi::xml_document doc;
	doc.load_string(body.c_str());

	SYSTEM_INFO info;
	GetNativeSystemInfo(&info);
	using winrt::Windows::System::ProcessorArchitecture;
	auto CurrArch = (ProcessorArchitecture)info.wProcessorArchitecture;

	std::map<ProcessorArchitecture, std::string_view> Arch2Str = {
		{ProcessorArchitecture::X86, "x86"},
		{ProcessorArchitecture::X64, "x64"},
		{ProcessorArchitecture::Arm, "arm"}
	};
	std::string_view CurrStrArch = Arch2Str[CurrArch];

	if (auto package = getPackageByFamilyName("24831TIRRSOFT.FS_7dqv9t6ww56qc")) {

		std::string_view version = doc.child("AppInstaller").attribute("Version").value();

		const auto& [major, minor, build, revision] = package->Id().Version();
		std::string localVersion = fmt::format("{}.{}.{}.{}", major, minor, build, revision);

		if (localVersion == version && package->Id().Architecture() == CurrArch) {
			gui::SetLabel("FS Client is up-to-dated");
			return true;
		}
	}

	for (pugi::xml_node package : doc.child("AppInstaller").child("Dependencies").children("Package")) {

		if (package.attribute("ProcessorArchitecture").value() == CurrStrArch) {

			std::string_view pkgName = package.attribute("Name").value();

			if (!isPackageExists(pkgName)) {

				gui::SetLabel(pkgName);

				auto status = installPackageByUrl(package.attribute("Uri").value());

				if (status == winrt::Windows::Foundation::AsyncStatus::Completed)
					gui::SetProgressStatic("Successfully installed.");
				else if (status == winrt::Windows::Foundation::AsyncStatus::Error)
					return false;
			}
		}
	}

	gui::SetLabel("Installing FS package...");

	auto status = installPackageByUrl(fmt::format("https://fsclient.github.io/fs/FSClient.UWP/FSClient.UWP_{}.appx", CurrStrArch));

	if (status == winrt::Windows::Foundation::AsyncStatus::Completed)
		gui::SetProgressStatic("Installation complete.");
	else if (status == winrt::Windows::Foundation::AsyncStatus::Error)
		return false;

	return true;
}