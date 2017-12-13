
#include <string>
#include <vector>

#include <winsock2.h>
#include <Iprtrmib.h>
#include <wsipx.h>
#include <wsnwlink.h>
#include <lm.h>

#include <Ws2tcpip.h>

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "Ws2_32.lib")




class SysInfo
{
public:
    std::wstring work_group;
    std::wstring computer_name;
    std::vector<std::string> local_ip_list;

    static SysInfo GetSystemInfo()
    {
        SysInfo SysInfo_;

        // getting IP address and subnet mask
        typedef DWORD(CALLBACK* GetIpAddr)(LPVOID, LPVOID, BOOL);

        HINSTANCE hDllHandle = NULL;
        GetIpAddr GetIpAddrPtr = NULL;

        // loading library for getting information
        hDllHandle = LoadLibrary(L"iphlpapi.dll");

        // library not loaded
        if (hDllHandle == NULL)
        {
            //fprintf(stderr, "Load iphlpapi.dll Error. IP address and subnetmask woun't be displayed.\n");
        }
        else
        {
            // getting the pointer to IP addresses
            GetIpAddrPtr = (GetIpAddr)GetProcAddress(hDllHandle, "GetIpAddrTable");
            // pointer not getted successfuly
            if (GetIpAddrPtr == NULL)
            {
                //fprintf(stderr, "Get GetIpAddrTable Error. IP address and subnetmask woun't be displayed.\n");
                // unloading the library
                FreeLibrary(hDllHandle);
            }
            else
            {
                char acAddr[255];
                DWORD dwSize = 255;
                DWORD dwRet = GetIpAddrPtr((LPVOID)&acAddr, &dwSize, FALSE);

                if (dwRet != NO_ERROR)
                {
                    //fprintf(stderr, "Error while getting IP address. Errorcode = %d. IP address and subnetmask woun't be displayed.\n", dwRet);
                    // unloading the library
                    FreeLibrary(hDllHandle);
                }
                else
                {
                    // unloading the library
                    FreeLibrary(hDllHandle);

                    MIB_IPADDRTABLE* pAddr = (MIB_IPADDRTABLE *)acAddr;
                    // getting a printing the IP address and subnet mask information
                    for (int i = 0; i < (int)pAddr->dwNumEntries; i++)
                    {
                        MIB_IPADDRROW *pTable = (MIB_IPADDRROW *)(acAddr + 4 + sizeof(MIB_IPADDRROW)*i);
                        //char acIp[16];
                        //char acMask[16];
                        char Dst[256];
                        const char *pChar = inet_ntop(AF_INET, &pTable->dwAddr, Dst, 256);
                        //memcpy(acIp, pChar, sizeof(acIp));
                        //pChar = inet_ntoa((in_addr &)pTable ->dwMask);
                        //memcpy(acMask, pChar, sizeof(acMask));

                        SysInfo_.local_ip_list.push_back(pChar);
                        //printf("Number of Adapter: %d\n\tIP\t: %s\n\tMask\t: %s\n", i, acIp, acMask);
                    }
                }
            }
        }

        DWORD dwLevel = 100;
        LPWKSTA_INFO_100 pBuf = NULL;
        NET_API_STATUS nStatus;
        LPTSTR pszServerName = NULL;

        // getting domain/workgroup name
        nStatus = NetWkstaGetInfo(pszServerName, dwLevel, (LPBYTE *)&pBuf);
        // If the call is successful, print the workstation data.
        if (nStatus == NERR_Success)
        {
            SysInfo_.work_group = pBuf->wki100_langroup;
            SysInfo_.computer_name = pBuf->wki100_computername;
            //wprintf(L"Workgroup or Domain: %s\n", pBuf ->wki100_langroup);	// workgroup or domain name
        }
        // indicate the system error.
        //else
        //	fprintf(stderr, "Error while getting domain/workgroup name. Errorcode = %d\n", nStatus);

        // Free the allocated memory.
        if (pBuf != NULL)
            NetApiBufferFree(pBuf);

        return SysInfo_;
    }
};

