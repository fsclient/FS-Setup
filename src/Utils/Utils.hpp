#pragma once

std::string httpGet(std::string_view url);

bool isPackageExists(std::string_view pkgName);
std::optional<winrt::Windows::ApplicationModel::Package> getPackageByFamilyName(std::string_view familyName);

winrt::Windows::Foundation::AsyncStatus installPackageByAppInstallerUrl(std::string_view url);
winrt::Windows::Foundation::AsyncStatus installPackageByUrl(std::string_view url);

bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string_view thumbPrint);

int getOsVersion();