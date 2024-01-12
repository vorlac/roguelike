#pragma once

#include <assert.h>

#ifdef _WIN32
  #ifndef LEAN_AND_MEAN
    #define LEAN_AND_MEAN
  #endif
  #include <Windows.h>
#endif

void get_ipv4(const char* const hostname, char* ipv4_addr, int buf_len)
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result == 0)
    {
        hostent* pHostInfo = ::gethostbyname(hostname);
        if (pHostInfo != nullptr)
        {
            in_addr* pINAddr = (in_addr*)(pHostInfo->h_addr_list[0]);
            if (pINAddr != nullptr)
            {
                char* ipv4 = inet_ntoa(*pINAddr);
                assert(buf_len > strlen(ipv4));
                strncpy(ipv4_addr, ipv4, strlen(ipv4));
            }
        }
        WSACleanup();
    }
}
