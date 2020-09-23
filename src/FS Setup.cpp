#include "pch.h"
#include "gui/GUI Utils.h"
#include "Utils/Utils.h"

void install_certificate();

DWORD WINAPI MainThread(HWND MainWindow) {

	install_certificate();
	ShellExecute(NULL, L"open", L"ms-appinstaller:?source=https://fsclient.github.io/fs/FSClient.UWP.appinstaller", NULL, NULL, SW_SHOWDEFAULT);

	SendMessage(MainWindow, WM_DESTROY, 0, 0);
	return 0;
}

void install_certificate() {

	gui::SetLabel(L"Installing certificate...");
	HCERTSTORE hRootCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING, NULL, CERT_SYSTEM_STORE_LOCAL_MACHINE, L"Root");

	if (!CheckCertByThumbPrint(hRootCertStore, "24542010cb06ad6ab320c84a984c7862e029fb08")) {

		httplib::Client Session("https://fsclient.github.io");
		auto res = Session.Get("fs/FSClient.UWP/FSClient.UWP.cer");

		PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING, (const BYTE*)res->body.c_str(), res->body.size() + 1);
		CertAddCertificateContextToStore(hRootCertStore, pCertContext, CERT_STORE_ADD_USE_EXISTING, NULL);
	}
	CertCloseStore(hRootCertStore, 0);
}