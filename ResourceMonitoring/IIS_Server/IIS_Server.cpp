// IIS_Server.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <iostream>
#include <numeric>

#define _NO_PPLXIMP
#define _NO_ASYNCRTIMP
//#define CPPREST_FORCE_PPLX
#define CPPREST_EXCLUDE_WEBSOCKETS

#include <cpprest/astreambuf.h>
#include <cpprest/filestream.h>
#include <cpprest/producerconsumerstream.h>

//#include "../PackageControlLib/WorkGroupMgr.h"
//#include "../PackageControlLib/regenum.h"
#include <ppl.h>
#include <cpprest/http_listener.h>

#pragma comment(lib, "Httpapi.lib")
#pragma comment(lib, "cpprestsdk120")

using namespace concurrency;
using namespace concurrency::streams;
using namespace web::http::client;

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

//  1. Installed software list &uninstall software
//		- Read registry installed software(agent)
//		- Post user pc alive(agent)
//		- Response installed software requested(server)
//		- Post installed software list to server(agent)
//		- Get user pc list(admin)
//		- Get user pc installed software list(admin)
//		- Get user pc installed software list rerequest(admin)
//		- Post user pc uninstall software(admin)


class WorkgroupManager
{
    struct ComputerInfo
    {
        utility::string_t GroupName;
        utility::string_t ComputerName;
        std::vector<utility::string_t> IpAddressList;
        utility::string_t SoftwareList;
    };
    typedef std::map<utility::string_t, std::shared_ptr<ComputerInfo>> ComputerMapT;
    typedef std::map<utility::string_t, ComputerMapT> WorkgroupMapT;

    WorkgroupMapT WorkgroupMap;
public:
    void Add(const utility::string_t &json_buffer)
    {
        json::value info = json::value::parse(json_buffer);

        auto cominfo = std::make_shared<ComputerInfo>();
        {
            cominfo->GroupName = info[U("Workgroup")].as_string();
            cominfo->ComputerName = info[U("Computername")].as_string();

            json::value iparr = info[U("IPList")];
            for (unsigned int i = 0; i<iparr.size(); i++)
                cominfo->IpAddressList.push_back(iparr[i].as_string());
            cominfo->SoftwareList = info[U("SoftwareList")].serialize();
        }
        WorkgroupMap[cominfo->GroupName][cominfo->ComputerName] = cominfo;
    }
    utility::string_t GetComputerList()
    {
        json::value grouplist;

        std::for_each(WorkgroupMap.begin(), WorkgroupMap.end(), [&grouplist](const WorkgroupMapT::value_type &g){
            json::value group;
            group[U("Groupname")] = json::value(g.first);
            json::value comlist;
            auto cm = g.second;
            for (auto it = cm.begin(); it != cm.end(); it++)
            {
                json::value item;

                item[U("Computername")] = json::value(it->second->ComputerName);
                comlist[comlist.size()] = item;
            };
            group[U("ComputerList")] = comlist;
            grouplist[grouplist.size()] = group;
        });
        json::value group_obj;
        group_obj[U("GroupList")] = grouplist;
        return grouplist.serialize();
    }
    utility::string_t GetSoftwareList(const utility::string_t &group_name, const utility::string_t &computer_name)
    {
        if (WorkgroupMap.find(group_name) != WorkgroupMap.end())
        {
            ComputerMapT &ComputerMap = WorkgroupMap[group_name];
            if (ComputerMap.find(computer_name) != ComputerMap.end())
            {
                std::shared_ptr<ComputerInfo> computer_ptr = ComputerMap[computer_name];
                return computer_ptr->SoftwareList;
            }
        }
        return U("");
    }
};
WorkgroupManager WorkgroupManager_;

template <class T, class A>
T join(const A &begin, const A &end, const T &t)
{
    T result;
    A it = begin;
    if (it != end)
        result.append(*it++);

    for (; it != end; ++it)
        result.append(t).append(*it);
    return result;
}

bool FileExists(const utility::string_t &fileName)
{
    DWORD       fileAttr;

    fileAttr = GetFileAttributes(fileName.c_str());
    if (0xFFFFFFFF == fileAttr && GetLastError() == ERROR_FILE_NOT_FOUND)
        return false;
    return true;
}

bool end_with(const utility::string_t &str, const utility::string_t &suffix)
{
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

typedef std::pair<utility::string_t, utility::string_t> string_pair;

std::vector<string_pair> content_type_array;

void init_content_type_array()
{
    content_type_array.push_back(std::make_pair<utility::string_t, utility::string_t>(L".htm", L"text/html"));
    content_type_array.push_back(std::make_pair<utility::string_t, utility::string_t>(L".html", L"text/html"));
    content_type_array.push_back(std::make_pair<utility::string_t, utility::string_t>(L".js", L"text/javascript"));
    content_type_array.push_back(std::make_pair<utility::string_t, utility::string_t>(L".css", L"text/css"));
    content_type_array.push_back(std::make_pair<utility::string_t, utility::string_t>(L".gif", L"image/gif"));
    content_type_array.push_back(std::make_pair<utility::string_t, utility::string_t>(L".png", L"image/png"));
    content_type_array.push_back(std::make_pair<utility::string_t, utility::string_t>(L".jpg", L"image/jpg"));
};

// 2013-07-05 : get image file implementation!
void response_file(const utility::string_t &file_path, const http_request &request)
{
    if (FileExists(file_path))
    {
        auto it = std::find_if(content_type_array.begin(), content_type_array.end(),
            [&file_path](const string_pair &p){ return end_with(file_path, p.first); });

        if (it != content_type_array.end())
        {
            streams::producer_consumer_buffer<uint8_t> buf;

            http_response response(200);

            //response.body().read_to_end(buf).get();
            response.set_body(buf.create_istream(), it->second);

            response.headers().add(header_names::connection, U("close"));
            request.reply(response);

            auto fileStream = fstream::open_istream(file_path, std::ios_base::binary).get();
            fileStream.read_to_end(buf).get();

            //buf.putn((const uint8_t *)responseData.data(), responseData.size()).get();
            buf.close(std::ios_base::out).get();
            fileStream.close().get();
        }
        else
        {
            request.reply(status_codes::UnsupportedMediaType);
        }
    }
    else
    {
        request.reply(status_codes::NotFound);
    }
}

// agent command
// http://localhost:8088/installed_software
//int main(int argc, char* argv[])
//int _tmain(int argc, _TCHAR* argv[])
int _tmain()
{
    init_content_type_array();

    std::wstring json_buffer;
    http_listener listener(L"http://localhost:8088/is");

    listener.support(methods::GET, [&](http_request request)
    {
        auto path_vector = uri::split_path(request.request_uri().path());

        // default request : initialize html
        if (path_vector.size() == 1)
        {
            std::for_each(request.headers().begin(), request.headers().end(),
                [](http_headers::const_reference cr)
            {
                std::wcout << cr.first << ":" << cr.second << std::endl;
            });

            response_file(L"installed_software_layout.html", request);
        }
        else
        {
            // request software list
            if (path_vector[1] == L"list")
            {

                request.reply(status_codes::OK, json_buffer);
            }
            else if (path_vector[1] == L"computerlist")
            {
                request.reply(status_codes::OK, WorkgroupManager_.GetComputerList());
            }
            else if (path_vector[1] == L"softwarelist")
            {
                std::wstring group_name = path_vector[2];
                std::wstring computer_name = path_vector[3];
                request.reply(status_codes::OK, WorkgroupManager_.GetSoftwareList(group_name, computer_name));
            }
            else
            {
                utility::string_t file_path = join<std::wstring>(path_vector.begin() + 1, path_vector.end(), L"/");
                response_file(file_path, request);
            }
        }
        std::wcout << "GET " << request.request_uri().to_string() << std::endl;
    });

    listener.support(methods::POST, [&](http_request request)
    {
        // split uri
        auto path_vector = uri::split_path(request.request_uri().path());

        if (path_vector.size() > 1)
        {
            // request software list
            if (path_vector[1] == L"software_list")
            {
                //request._get_impl().
                //std::wcout << request.to_string();
                //std::for_each(request.headers().begin(), request.headers().end(),
                //	[](http_headers::const_reference cr)
                //{
                //	std::wcout << cr.first << ":" << cr.second << std::endl;
                //});
                json_buffer = request.extract_string().get();

                WorkgroupManager_.Add(json_buffer);

                //request.reply(status_codes::OK);

                http_response response(status_codes::OK);
                response.headers().add(header_names::connection, U("close"));
                request.reply(response);

                //std::cout << "POST Request..." << std::endl;
            }
        }
    });

    listener.support(methods::PUT, [=](http_request request)
    {
        request.reply(status_codes::NoContent);
        std::cout << "PUT Request..." << std::endl;
    });

    listener.support(methods::PUT, [=](http_request request)
    {
        request.reply(status_codes::NoContent);
        std::cout << "DELETE Request..." << std::endl;
    });

    /* Prevent Listen() from returning until user hits 'Enter' */

    try
    {
        listener
            .open()
            .then([&listener](){std::wcout << std::endl << L"starting to listen" << std::endl; })
            .wait();

        while (true);
    }
    catch (std::exception const & e)
    {
        std::wcout << e.what() << std::endl;
    }

    return 0;
}


/*
#include <http_client.h>
#include <filestream.h>
#include <uri.h>
#include <thread>

#pragma comment(lib, "casablanca110")

using namespace concurrency::streams;
using namespace web::http::client;
using namespace web::http;

int main()
{
// Open stream to file.
file_stream<unsigned char>::open_istream(L"ReadMe.txt").then([](basic_istream<unsigned char> fileStream)
{
// Make HTTP request with the file stream as the body.
http_client client(U("http://www.myhttpserver.com"));
client.request(methods::PUT, L"myfile", fileStream).then([fileStream](http_response response)
{
fileStream.close();
// Perform actions here to inspect the HTTP response...
if (response.status_code() == status_codes::OK)
{
}
});
});
std::this_thread::sleep_for(std::chrono::seconds(10));
return 0;
}
*/

