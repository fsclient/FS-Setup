#pragma once

std::string ByteArrayToString(std::unique_ptr<BYTE[]>, size_t len);
bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string thumbPrint);

std::optional<std::string> getFullNameByFamilyName(std::string familyName);

DWORD getOsVersion();