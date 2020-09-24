#include "pch.h"
#include "Utils.h"

std::optional<std::string> getFullNameByFamilyName(std::string familyName) {

	UINT32 count = 0;
	UINT32 length = 0;

	std::wstring wFamilyName = boost::nowide::widen(familyName);

	LONG status = FindPackagesByPackageFamily(wFamilyName.c_str(), PACKAGE_FILTER_HEAD, &count, nullptr, &length, nullptr, nullptr);
	if (status == ERROR_SUCCESS || status != ERROR_INSUFFICIENT_BUFFER) return std::nullopt;

	auto fullNames = std::make_unique<PWSTR[]>(count * sizeof(PWSTR));
	if (fullNames == nullptr) return std::nullopt;

	auto buffer = std::make_unique<WCHAR[]>(length * sizeof(WCHAR));
	if (buffer == nullptr) return  std::nullopt;

	status = FindPackagesByPackageFamily(wFamilyName.c_str(), PACKAGE_FILTER_HEAD, &count, fullNames.get(), &length, buffer.get(), nullptr);
	if (status != ERROR_SUCCESS) return std::nullopt;

	return std::string(boost::nowide::narrow(fullNames[0]));
}

bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string thumbPrint) {

	PCCERT_CONTEXT pCertContext = NULL;

	while (pCertContext = CertEnumCertificatesInStore(hRootCertStore, pCertContext)) {

		DWORD length = 0;

		CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, nullptr, &length);

		auto buffer = std::make_unique<BYTE[]>(length);
		if (buffer == nullptr) return false;

		CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, buffer.get(), &length);

		if (ByteArrayToString(std::move(buffer)) == thumbPrint) return true;
	}
	return false;
}

#pragma warning(push)
#pragma warning(disable : 6387)
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
#pragma warning(pop)

std::string ByteArrayToString(std::unique_ptr<BYTE[]> BytePtr) {

	std::string hexString;

	boost::algorithm::hex(BytePtr.get(), back_inserter(hexString));
	boost::algorithm::to_lower(hexString);

	return hexString;
}