#pragma once

#include "../Util.h"

#include <unistd.h>
#include <sys/sysinfo.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

namespace Util
{
	std::vector<NetworkInterface> ListInterfaces()
	{
		using namespace std;
		std::vector<NetworkInterface> interfaces{};

		struct ifaddrs *ifAddrStruct = nullptr;
		struct ifaddrs *ifa = nullptr;
		void *tmpAddrPtr = nullptr;

		if (getifaddrs(&ifAddrStruct) == 0)
			for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
			{
				if (ifa->ifa_addr != nullptr && (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6) && // the interface has a valid IP4 Address
				    (ifa->ifa_flags & (IFF_RUNNING)) &&                                                                          // the interface is active
				    ! (ifa->ifa_flags & (IFF_LOOPBACK))                                                                          // the interface is not loopback
				)
				{
					if (ifa->ifa_addr->sa_family == AF_INET)
					{
						tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
						char addressBuffer[INET_ADDRSTRLEN] = {0};
						inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
						int firstOctet = atoi(addressBuffer);
						if (firstOctet > 0 && firstOctet != 127)
                            interfaces.push_back(NetworkInterface{std::string{addressBuffer}, std::string{ifa->ifa_name}, true, false});
					}
					else if (ifa->ifa_addr->sa_family == AF_INET6)
					{
						tmpAddrPtr = &((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr;
						char addressBuffer[INET6_ADDRSTRLEN] = {0};
						if (inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN))
                            interfaces.push_back(NetworkInterface{std::string{addressBuffer}, std::string{ifa->ifa_name}, false, false});
					}
				}
			}

		if (ifAddrStruct != nullptr)
			freeifaddrs(ifAddrStruct);

		interfaces.push_back(std::move(NetworkInterface{"127.0.0.1", "Localhost", true, true}));
        return interfaces;
	}
} // namespace Util
