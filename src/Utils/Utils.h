#pragma once

std::string ByteArrayToString(BYTE* BytePtr, size_t len);
bool CheckCertByThumbPrint(HCERTSTORE hRootCertStore, std::string thumbPrint);