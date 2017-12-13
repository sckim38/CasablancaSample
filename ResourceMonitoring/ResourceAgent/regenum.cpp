#include "stdafx.h"

#define _NO_PPLXIMP
#define _NO_ASYNCRTIMP
//#define CPPREST_FORCE_PPLX
#define CPPREST_EXCLUDE_WEBSOCKETS

#include <memory>
#include "regenum.h"
#include "sysinfo.h"


#include "cpprest\details\basic_types.h"

#include "pplx\pplxtasks.h"

#include <cpprest\http_client.h>

// Stream
#include "cpprest\streams.h"
#include "cpprest\astreambuf.h"

#include "cpprest\asyncrt_utils.h"

// json
#include "cpprest\json.h"

// uri
#include "cpprest\base_uri.h"
#include "cpprest\details\uri_parser.h"

// http
#include "cpprest\http_msg.h"
#include "cpprest\http_client.h"
#include "cpprest\details\http_helpers.h"

#pragma comment(lib, "Httpapi.lib")
#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "cpprestsdk120")

using namespace web;
using namespace web::http;
using namespace web::http::client;

std::wstring to_wstring(const std::string& s)
{
	return utility::conversions::utf8_to_utf16(s);
}

std::string utf16_to_usascii(const std::wstring& s)
{
    int slength = (int)s.length();// + 1;
    int len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0); 
    std::string r(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0); 
    return r.data();
}

std::wstring RegistryQueryValue(HKEY hKey, const wchar_t * szName)
{
    std::wstring value;

    DWORD dwType;
    DWORD dwSize = 0;

    if (::RegQueryValueEx(
        hKey,                   // key handle
        szName,                 // item name
        NULL,                   // reserved
        &dwType,                // type of data stored
        NULL,                   // no data buffer
        &dwSize                 // required buffer size
        ) == ERROR_SUCCESS && dwSize > 0)
    {
        value.resize(dwSize);

        ::RegQueryValueEx(
            hKey,                   // key handle
            szName,                 // item name
            NULL,                   // reserved
            &dwType,                // type of data stored
            (LPBYTE)&value[0],      // data buffer
            &dwSize                 // available buffer size
            );
    }
    return value.data();
}


DWORD RegistryQueryValueDWord(HKEY hKey, const wchar_t * szName)
{
    DWORD dwValue=0;

    DWORD dwType;
    DWORD dwSize = 0;

    if (::RegQueryValueEx(
        hKey,                   // key handle
        szName,                 // item name
        NULL,                   // reserved
        &dwType,                // type of data stored
        NULL,                   // no data buffer
        &dwSize                 // required buffer size
        ) == ERROR_SUCCESS && dwSize > 0)
    {
        ::RegQueryValueEx(
            hKey,                   // key handle
            szName,                 // item name
            NULL,                   // reserved
            &dwType,                // type of data stored
            (LPBYTE)&dwValue,      // data buffer
            &dwSize                 // available buffer size
            );
    }
    return dwValue;
}

struct CProgram
{
    std::wstring Key;

	std::wstring DisplayName;
	std::wstring Publisher;
	std::wstring DisplayVersion;
	//wstring InstallLocation = RegistryQueryValue(hItem, L"InstallLocation");
	DWORD EstimatedSize;
    std::wstring UninstallString;
    std::wstring QuietUninstallString;
};

typedef std::map<std::wstring, std::shared_ptr<CProgram>> ProgramListT;
ProgramListT ProgramList;

static json::value ReadInstalledSoftware()
{
	ProgramList.clear();

    HKEY Roots[2] = {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE};

	for(int i=0; i<2; i++)
	{
		HKEY hKey;
		LONG ret = ::RegOpenKeyEx(
			Roots[i], //HKEY_LOCAL_MACHINE,     // local machine hive
			U("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), // uninstall key
			0,                      // reserved
			KEY_READ,               // desired access
			&hKey                   // handle to the open key
			);

		if(ret != ERROR_SUCCESS)
			continue;

		DWORD dwIndex = 0;
		DWORD cbName = 1024;
		TCHAR szSubKeyName[1024];

		while ((ret = ::RegEnumKeyEx(
			hKey, 
			dwIndex, 
			szSubKeyName, 
			&cbName, 
			NULL,
			NULL, 
			NULL, 
			NULL)) != ERROR_NO_MORE_ITEMS)
		{
			if (ret == ERROR_SUCCESS)
			{
				HKEY hItem;
				if (::RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_READ, &hItem) != ERROR_SUCCESS)
					continue;

                std::wstring UninstallString = RegistryQueryValue(hItem, U("UninstallString"));
                std::wstring QuietUninstallString = RegistryQueryValue(hItem, U("QuietUninstallString"));

				if (!UninstallString.empty() || !QuietUninstallString.empty())
				{
                    std::shared_ptr<CProgram> program = std::make_shared<CProgram>();

					program->Key = szSubKeyName;
					program->DisplayName = RegistryQueryValue(hItem, U("DisplayName"));
					program->Publisher = RegistryQueryValue(hItem, U("Publisher"));
					program->DisplayVersion = RegistryQueryValue(hItem, U("DisplayVersion"));
					//wstring InstallLocation = RegistryQueryValue(hItem, L"InstallLocation");
					program->EstimatedSize = RegistryQueryValueDWord(hItem, U("EstimatedSize"));
					program->UninstallString = UninstallString;
					program->QuietUninstallString = QuietUninstallString;

					ProgramList[program->Key] = program;
				}
				::RegCloseKey(hItem);
			}
			dwIndex++;
			cbName = 1024;
		}
		::RegCloseKey(hKey);
	}

	// make software list to json
	json::value arr;
	for_each(ProgramList.begin(), ProgramList.end(), [&arr](ProgramListT::value_type item)
	{
		auto spec = item.second;
		json::value obj;

		obj[U("Key")] = json::value(spec->Key);
		obj[U("DisplayName")] = json::value(spec->DisplayName);
		obj[U("Publisher")] = json::value(spec->Publisher);
		obj[U("DisplayVersion")] = json::value(spec->DisplayVersion);
		obj[U("EstimatedSize")] = json::value((int)spec->EstimatedSize);

		arr[arr.size()] = obj;
	});
	return arr;
}

std::wstring InstalledsoftwareToJson()
{
	return ReadInstalledSoftware().serialize();
}

std::wstring GetSWInfoToJsonFormat()
{
	// get computer info(workgroup & name)
	SysInfo sysinfo = SysInfo::GetSystemInfo();
	json::value info;

	info[U("Workgroup")] = json::value(sysinfo.work_group);
	info[U("Computername")] = json::value(sysinfo.computer_name);

	json::value iparr;
	std::for_each(sysinfo.local_ip_list.begin(), sysinfo.local_ip_list.end(), [&iparr](const std::string &ip)
	{
		iparr[iparr.size()] = json::value(::to_wstring(ip));
	});

	info[U("IPList")] = iparr;
	info[U("SoftwareList")] = ReadInstalledSoftware();

	//ofstream wfs("aaa.txt");
	//wfs << utf16_to_usascii(info.to_string());

	return info.serialize();
}

std::wstring GetSWInfoToJsonFormat(const std::wstring &work_group, const std::wstring &computer_name)
{
	// get computer info(workgroup & name)
	SysInfo sysinfo = SysInfo::GetSystemInfo();
	json::value info;

	info[U("Workgroup")] = json::value(work_group);
	info[U("Computername")] = json::value(computer_name);

	json::value iparr;
    std::for_each(sysinfo.local_ip_list.begin(), sysinfo.local_ip_list.end(), [&iparr](const std::string &ip)
	{
		iparr[iparr.size()] = json::value(to_wstring(ip));
	});

	info[U("IPList")] = iparr;
	info[U("SoftwareList")] = ReadInstalledSoftware();

	//ofstream wfs("aaa.txt");
	//wfs << utf16_to_usascii(info.to_string());

	return info.serialize();
}


//utility::string_t ParseTest()
//{
//	WorkgroupManager WorkgroupManager_;
//    std::wstring json_str = GetSWInfoToJsonFormat();
//	WorkgroupManager_.Add(json_str);
//
//	return WorkgroupManager_.GetComputerList();
//}

//______________________________________________________________________________
// Executes a command line (Exec) in a new process
void ExecuteString(const std::wstring Exec)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    TCHAR pExec[MAX_PATH];
    lstrcpy(pExec, Exec.c_str());

    // Start the child process.
    if(!CreateProcess(NULL, pExec, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        //ErrorBox("Couldn't launch the program \"" + Exec + "\"");
        return;
    }

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}

//______________________________________________________________________________
// Uninstall the selected program
void RemoveProgram(const std::wstring Key)
{
    auto v = ProgramList.find(Key);
    if (v != ProgramList.end())
    {
		auto p = v->second;
        std::wstring sQuietUninstall;

		if (!p->QuietUninstallString.empty())
            sQuietUninstall = p->QuietUninstallString;
        else //if (p->WindowsInstaller)
            sQuietUninstall = L"msiexec /quiet /uninstall " + p->Key;

        ExecuteString(sQuietUninstall);
	}
}

std::wstring PostInstalledSoftwareList(const std::wstring &rest_server/*=L"localhost:8088"*/)
{
	http_client client(L"http://" + rest_server + L"/is/software_list");

	//client.request(methods::GET).then([&](http_response response)
	return client.request(methods::POST, L"is", GetSWInfoToJsonFormat())
		.then([](http_response response)->std::wstring
		{
			//std::wstring status = result = to_wstring((_ULonglong)response.status_code());
			if(response.status_code() == status_codes::OK)
			{
				//response.extract_json().then([](json::value jsonValue)
				//{
				//	for(auto iterArray = jsonValue.cbegin(); iterArray != jsonValue.cend(); ++iterArray)
				//	{
				//		const json::value &arrayValue = iterArray->second;

				//		for(auto iterInner = arrayValue.cbegin(); iterInner != arrayValue.cend(); ++iterInner)
				//		{
				//			const json::value &propertyName = iterInner->first;
				//			const json::value &propertyValue = iterInner->second;

				//			std::wcout 
				//				<< L"Property: " << propertyName.as_string() 
				//				<< L", Value: " << propertyValue.to_string() 
				//				<< std::endl;

				//		}       

				//		std::wcout << std::endl;
				//	}
				//});
			}
			//return response.extract_string().get();
			//return utility::conversions::print_string( response.status_code() );
			return response.to_string();
		}).get();
}

void PostInstalledSoftwareList(
    const std::wstring &work_group,
    const std::wstring &computer_name,
    const std::wstring &rest_server/*=L"localhost:8088"*/)
{
	http_client client(L"http://" + rest_server + L"/is/software_list");

	//client.request(methods::GET).then([&](http_response response)
	client.request(methods::POST, L"is", GetSWInfoToJsonFormat(work_group, computer_name))
	.then([](http_response response)
	{
		//std::wstring status = result = to_wstring((_ULonglong)response.status_code());
		if(response.status_code() == status_codes::OK)
		{
		}
	}).get();
}


//
//#include <http_client.h>
//#include <filestream.h>
//#include <uri.h>
//#include <thread>
// 
//#pragma comment(lib, "casablanca110")
// 
//using namespace concurrency::streams;
//using namespace web::http::client;
//using namespace web::http;
//  
//int main()
//{
//  // Open stream to file.
//  file_stream<unsigned char>::open_istream(L"ReadMe.txt").then([](basic_istream<unsigned char> fileStream)
//  {
//    // Make HTTP request with the file stream as the body.
//    http_client client(U("http://www.myhttpserver.com"));
//    client.request(methods::PUT, L"myfile", fileStream).then([fileStream](http_response response)
//    {
//      fileStream.close();
//      // Perform actions here to inspect the HTTP response...
//      if (response.status_code() == status_codes::OK)
//      {
//      }
//    });
//  });
//  std::this_thread::sleep_for(std::chrono::seconds(10));
//  return 0;
//}
//



//MyListener::MyListener(const http::uri& url) :
//        m_listener(http_listener::create(url))
//    {
//        m_listener.support(methods::GET,
//                           std::tr1::bind(&MyListener::handle_get,
//                                          this,
//                                          std::tr1::placeholders::_1));
//        m_listener.support(methods::PUT,
//                           std::tr1::bind(&MyListener::handle_put,
//                                          this,
//                                          std::tr1::placeholders::_1));
//        m_listener.support(methods::POST,
//                           std::tr1::bind(&MyListener::handle_post,
//                                          this,
//                                          std::tr1::placeholders::_1));
//        m_listener.support(methods::DEL,
//                           std::tr1::bind(&MyListener::handle_delete,
//                                          this,
//                                          std::tr1::placeholders::_1));
//    }



//int _tmain(int argc, _TCHAR* argv[])
//{
//    uri_builder uri(L"http://localhost:4999/");
//    http_listener listener;
//    auto p = new Person("Filip","W",95);
//
//    listener = http_listener::create(uri.to_uri(),
//        [p](http_request message)
//        {
//        std::cout << "Serving GET" << std::endl;
//        message.reply(http::status_codes::OK, p->ToJSON());
//        },
//        [p](http_request message)
//        {
//        auto posted = p->FromJSON(message.extract_json().get());
//        message.reply(http::status_codes::NoContent);
//        std::cout << "Received a PUT of " << posted.FirstName << ", " << posted.LastName << ", " << posted.Age << std::endl;
//        },
//        [p](http_request message)
//        {
//        auto posted = p->FromJSON(message.extract_json().get());
//        std::cout << "Received a POST of " << posted.FirstName << ", " << posted.LastName << ", " << posted.Age << std::endl;
//        message.reply(http::status_codes::NoContent);
//        },
//        [p](http_request message)
//        {
//        message.reply(http::status_codes::NoContent);
//        std::cout << "Deleting " << p->FirstName << std::endl;
//        });
//
//    listener.listen([]() { fgetc(stdin); }).wait();
//    return 0;
//}

