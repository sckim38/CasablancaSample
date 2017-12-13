#include <string>

std::wstring to_wstring(const std::string& s);
std::string utf16_to_usascii(const std::wstring& s);
std::wstring InstalledsoftwareToJson();
std::wstring GetSWInfoToJsonFormat();
std::wstring GetSWInfoToJsonFormat(const std::wstring &work_group, const std::wstring &computer_name);

std::wstring ParseTest();

std::wstring GetComList();
void ExecuteString(const std::wstring Exec);
void RemoveProgram(const std::wstring Key);
std::wstring PostInstalledSoftwareList(const std::wstring &rest_server=(L"localhost:8088"));
void PostInstalledSoftwareList(const std::wstring &work_group, const std::wstring &computer_name, const std::wstring &rest_server=(L"localhost:8088"));
