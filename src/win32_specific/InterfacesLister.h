#pragma once

#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <map>
#include "../Util.h"
#pragma comment(lib, "iphlpapi.lib")

namespace Util {
	std::vector<NetworkInterface> ListInterfaces()
	{
	using namespace std;
	std::vector<NetworkInterface> interfaces;

	
    // use the new api for the ipv6 interfaces
    struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	struct addrinfo *result = nullptr;
	DWORD dwRetval = getaddrinfo("", nullptr, &hints, &result);
	if (dwRetval != 0)
		return interfaces;

	char buffer[64];
	struct addrinfo *ptr = nullptr;
	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
        if(ptr->ai_family == AF_INET6)
		{
			sockaddr_in6 *sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
			if (inet_ntop(ptr->ai_family, (const void *) (&sockaddr_ipv6->sin6_addr), buffer, 64))
			{
                auto item = NetworkInterface{std::string(buffer), "Unknown", false, false};
				interfaces.push_back(std::move(item));
			}				
		
        }
	}
	freeaddrinfo(result);

    // use the old windows api for ipv4, because with the new one i have no idea how to get the interface names
	ULONG buflen = sizeof(IP_ADAPTER_INFO);
	IP_ADAPTER_INFO *pAdapterInfo = (IP_ADAPTER_INFO *) malloc(buflen);

	if (GetAdaptersInfo(pAdapterInfo, &buflen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc(buflen);
	}

	if (GetAdaptersInfo(pAdapterInfo, &buflen) == NO_ERROR)
	{
		for (IP_ADAPTER_INFO *pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next)
		{
			int firstOctet = atoi(pAdapter->IpAddressList.IpAddress.String);
			if (firstOctet > 0 && firstOctet != 127)
			{
				auto item = NetworkInterface{std::string(pAdapter->IpAddressList.IpAddress.String), std::string(pAdapter->Description), true, false};
				interfaces.push_back(std::move(item));
			}
		}
	}

	// add a loopback interface
	interfaces.push_back(NetworkInterface{"127.0.0.1", "Localhost", true, true});

	if (pAdapterInfo)
		free(pAdapterInfo);

	return std::move(interfaces);
}
}  // namespace Util
