#pragma once
#include <fmt/format.h>
#include <iostream>
#include <vector>

namespace Util {
struct NetworkInterface {
    std::string address;
    std::string name;
    bool ipv4;
    bool loopback;

    bool operator<(const NetworkInterface &other) const {
        int cmp = address.compare(other.address);
        if (cmp != 0)
            return cmp < 0;
		if (ipv4 != other.ipv4)
            return ipv4 < other.ipv4;
        cmp = name.compare(other.name);
        if (cmp != 0)
            return cmp < 0;
        if (loopback != other.loopback)
            return loopback < other.loopback;
        return false;
    }

    std::string to_string() const {
        return fmt::format("{{ addr: {0}, name: '{3}', ipv4: {1}, loopback: {2} }}", address, ipv4, loopback, name);
    }
};

std::string RandomizePath(const std::string &path){
    auto now = std::chrono::system_clock::now();
    std::hash<std::string> hash_function;
    return std::move(fmt::sprintf("%x", hash_function(path) ^ std::time(0)));
}

}  // namespace Util

#ifdef _WIN32
	#include "./win32_specific/InterfacesLister.h"
#else
	#include "./posix_specific/InterfacesLister.h"
#endif