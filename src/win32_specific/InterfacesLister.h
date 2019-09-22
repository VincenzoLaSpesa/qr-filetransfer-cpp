#pragma once

#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include "../Util.h"
#pragma comment(lib, "iphlpapi.lib")

namespace Util {
std::vector<NetworkInterface> ListInterfaces() {
    std::map<asio::ip::address, NetworkInterface> container;
    std::vector<NetworkInterface> i;
    asio::io_service io_service;

    asio::ip::tcp::resolver resolver(io_service);
    asio::ip::tcp::resolver::query query(asio::ip::host_name(), "");
    asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
    if (it != asio::ip::tcp::resolver::iterator())
        do {
            const auto endpoint = it->endpoint();
            const auto hostname = it->host_name();
            const asio::ip::address addr = endpoint.address();
            container.insert(std::pair<asio::ip::address, NetworkInterface>(addr, NetworkInterface{addr.to_string(), "Unknown", addr.is_v4(), addr.is_loopback()}));
        } while (++it != asio::ip::tcp::resolver::iterator());
	// try to get the names with windows API
    ULONG buflen = sizeof(IP_ADAPTER_INFO);
    IP_ADAPTER_INFO *pAdapterInfo = (IP_ADAPTER_INFO *)malloc(buflen);

    if (GetAdaptersInfo(pAdapterInfo, &buflen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)malloc(buflen);
    }

    if (GetAdaptersInfo(pAdapterInfo, &buflen) == NO_ERROR) {
        for (IP_ADAPTER_INFO *pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next) {
            const asio::ip::address addr = asio::ip::address::from_string(pAdapter->IpAddressList.IpAddress.String);
            auto it = container.find(addr);
            if (it != container.end()) 
                container.at(addr).name = std::string(pAdapter->Description);
        }
    }

    if (pAdapterInfo)
        free(pAdapterInfo);

    for (auto const &imap : container)
        i.push_back(std::move(imap.second));
    i.push_back(std::move(NetworkInterface{"0.0.0.0", "This host on this network", true, false}));
    i.push_back(std::move(NetworkInterface{"127.0.0.1", "Localhost", true, true}));
	
	return std::move(i);
}
}  // namespace Util
