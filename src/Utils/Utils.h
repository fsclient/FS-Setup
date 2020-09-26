#pragma once

boost::filesystem::path createFile(boost::filesystem::path filepath, std::string data);

std::string ByteArrayToString(std::unique_ptr<BYTE[]>);
bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string thumbPrint);

std::optional<std::string> getFullNameByFamilyName(std::string familyName);

DWORD getOsVersion();