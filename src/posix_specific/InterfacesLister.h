#pragma once

#include "../Util.h"

namespace Util {
std::vector<NetworkInterface> ListInterfaces() {
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
            i.push_back(NetworkInterface{addr.to_string(), "Unknown", addr.is_v4(), addr.is_loopback()});
        } while (++it != asio::ip::tcp::resolver::iterator());
    i.push_back(std::move(NetworkInterface{"127.0.0.1", "Localhost", true, true}));        
    return std::move(i);
}
}  // namespace Util
