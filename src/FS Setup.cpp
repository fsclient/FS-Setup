#include "pch.h"
#include "gui/GUI Utils.h"
#include "Utils/Utils.h"

void install_certificate();

DWORD WINAPI MainThread(HWND MainWindow) {

	DWORD minOsVer = 16299;

	if (getOsVersion() >= minOsVer) {

		install_certificate();

		if (getFullNameByFamilyName("Microsoft.DesktopAppInstaller_8wekyb3d8bbwe")) {

			ShellExecute(NULL, L"open", L"ms-appinstaller:?source=https://fsclient.github.io/fs/FSClient.UWP.appinstaller", NULL, NULL, SW_SHOWDEFAULT);
		}	
		else MessageBox(MainWindow, L"You haven't app installer in your system.", L"Error", MB_ICONERROR | MB_OK);
	}
	else MessageBox(MainWindow, L"Your OS Version isn't supported.", L"Error", MB_ICONERROR | MB_OK);
	
	SendMessage(MainWindow, WM_DESTROY, 0, 0);
	return 0;
}

void install_certificate() {

	gui::SetLabel("Installing certificate...");
	gui::SetPending(true);

	HCERTSTORE hRootCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING, NULL, CERT_SYSTEM_STORE_LOCAL_MACHINE, L"Root");

	if (!CheckCertByThumbPrint(hRootCertStore, "24542010cb06ad6ab320c84a984c7862e029fb08")) {

		httplib::Client Session("https://fsclient.github.io");
		auto res = Session.Get("fs/FSClient.UWP/FSClient.UWP.cer");

		PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING, (const BYTE*)res->body.c_str(), (DWORD)res->body.size() + 1);
		CertAddCertificateContextToStore(hRootCertStore, pCertContext, CERT_STORE_ADD_USE_EXISTING, NULL);
	}
	CertCloseStore(hRootCertStore, 0);
}