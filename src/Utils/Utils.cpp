#include "pch.h"
#include "Utils.h"

bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string thumbPrint) {

	PCCERT_CONTEXT pCertContext = NULL;

	while (pCertContext = CertEnumCertificatesInStore(hRootCertStore, pCertContext)) {

		DWORD pcbHash = 20;
		BYTE* pbHash = new BYTE[pcbHash];

		CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, pbHash, &pcbHash);

		std::string sha1hash = ByteArrayToString(pbHash, pcbHash);
		delete[] pbHash;

		if (sha1hash == thumbPrint) return true;
	}
	return false;
}

std::string ByteArrayToString(BYTE* BytePtr, size_t len)
{
	std::stringstream ss;
	ss << std::hex;

	for (size_t i(0); i < len; ++i)
		ss << std::setw(2) << std::setfill('0') << (int)BytePtr[i];

	return ss.str();
}