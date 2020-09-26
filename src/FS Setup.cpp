#include "pch.h"
#include "gui/GUI Utils.h"
#include "Utils/Utils.h"

void install_certificate();
void manualInstall();

DWORD WINAPI MainThread(HWND MainWindow) {

	DWORD minOsVer = 16299;

	if (getOsVersion() >= minOsVer) {

		install_certificate();

		if (getFullNameByFamilyName("Microsoft.DesktopAppInstaller_8wekyb3d8bbwe")) {

			ShellExecute(NULL, L"open", L"ms-appinstaller:?source=https://fsclient.github.io/fs/FSClient.UWP.appinstaller", NULL, NULL, SW_SHOWDEFAULT);
		}	
		else {
			
			manualInstall();

			gui::SetPending(false);
			gui::SetFullProgress();

			using namespace std::chrono_literals;

			std::this_thread::sleep_for(3s);
		}
	}
	else MessageBox(MainWindow, L"Your OS Version isn't supported.", L"Error", MB_ICONERROR | MB_OK);
	
	SendMessage(MainWindow, WM_DESTROY, 0, 0);
	return 0;
}

void manualInstall() {

	gui::SetLabel("Trying to manual install...");

	web::http::client::http_client client(U("https://fsclient.github.io"));
	auto response = client.request(web::http::methods::GET, U("/fs/FSClient.UWP.appinstaller")).get();
	std::string body = response.extract_utf8string(true).get();

	tinyxml2::XMLDocument doc;
	doc.Parse(body.c_str(), body.size());

	if (auto fullName = getFullNameByFamilyName("24831TIRRSOFT.FS_7dqv9t6ww56qc")) {

		std::string version = doc.FirstChildElement("AppInstaller")->Attribute("Version");

		std::vector<std::string> strings;
		boost::split(strings, fullName.value(), boost::is_any_of("_"));

		std::string localVersion = strings[1];
		std::string pkgArchitecture = strings[2];

		if (localVersion == version && pkgArchitecture == "x64") {
			gui::SetLabel("FS Client is up-to-dated");
				return;
		}
	}

	boost::filesystem::path tempDir = boost::filesystem::temp_directory_path() /= boost::filesystem::unique_path();
	boost::filesystem::create_directory(tempDir);

	auto tempFilePath = createFile(tempDir /= "FS.appinstaller", body);

	//PowerShell.exe -Command "Add-AppxPackage 'path' -AppInstallerFile"
	std::string command = (boost::format{ "-Command \"Add-AppxPackage '%s' -AppInstallerFile\"" } % tempFilePath).str();

	if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE) == S_OK) {

		SHELLEXECUTEINFO ShExecInfo;

		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = L"open";
		ShExecInfo.lpFile = L"PowerShell.exe";
		ShExecInfo.lpParameters = boost::nowide::widen(command).c_str();
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_SHOWDEFAULT;
		ShExecInfo.hInstApp = NULL;

		ShellExecuteEx(&ShExecInfo);
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
		CloseHandle(ShExecInfo.hProcess);

		gui::SetLabel("Installation was completed.");
	}

	boost::filesystem::remove_all(tempDir);
}

void install_certificate() {

	gui::SetLabel("Installing certificate...");
	gui::SetPending(true);

	HCERTSTORE hRootCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING, NULL, CERT_SYSTEM_STORE_LOCAL_MACHINE, L"Root");

	if (CheckCertByThumbPrint(hRootCertStore, "24542010cb06ad6ab320c84a984c7862e029fb08")) {

		web::http::client::http_client client(U("https://fsclient.github.io"));
		auto response = client.request(web::http::methods::GET, U("fs/FSClient.UWP/FSClient.UWP.cer")).get();
		std::string body = response.extract_utf8string(true).get();

		PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING, (const BYTE*)body.c_str(), (DWORD)body.size() + 1);
		CertAddCertificateContextToStore(hRootCertStore, pCertContext, CERT_STORE_ADD_USE_EXISTING, NULL);
	}
	CertCloseStore(hRootCertStore, 0);
}