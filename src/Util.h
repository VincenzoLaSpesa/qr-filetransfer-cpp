#pragma once
#include <fmt/format.h>
#include <iostream>
#include <vector>

namespace Util {
struct NetworkInterface {
    std::string address;
    bool ipv4;
    bool loopback;

    bool operator<(const NetworkInterface &other) const {
        if (ipv4 != other.ipv4)
            return ipv4 > other.ipv4;
        if (loopback != other.loopback)
            return loopback > other.loopback;

        return address.compare(other.address) < 0;
    }

    std::string to_string() const {
        return fmt::format("{{ addr: {0}, ipv4: {1}, loopback: {2} }}", address, ipv4, loopback);
    }
};

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
            i.push_back(NetworkInterface{addr.to_string(), addr.is_v4(), addr.is_loopback()});
        } while (++it != asio::ip::tcp::resolver::iterator());
    return std::move(i);
}

std::string RandomizePath(const std::string &path){
    auto now = std::chrono::system_clock::now();
    std::hash<std::string> hash_function;
    return std::move(fmt::sprintf("%x", hash_function(path) ^ std::time(0)));
}

}  // namespace Util
