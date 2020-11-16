#pragma once

std::string httpGet(std::string_view url);

[[deprecated]] bool installPackageByPath(std::filesystem::path filePath);
bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string_view thumbPrint);

bool isPackageExists(std::string_view pkgName);
std::optional<winrt::Windows::ApplicationModel::Package> getPackageByFamilyName(std::string_view familyName);

int getOsVersion();