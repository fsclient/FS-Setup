#pragma once

std::filesystem::path createFile(std::filesystem::path filepath, std::string data);
std::string httpGet(std::string_view url);

bool installPackageByPath(std::filesystem::path filePath);

std::string ByteArrayToString(std::unique_ptr<BYTE[]>);
bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string thumbPrint);

bool isPackageExists(std::string_view pkgName);
std::optional<winrt::Windows::ApplicationModel::Package> getPackageByFamilyName(std::string_view familyName);

DWORD getOsVersion();