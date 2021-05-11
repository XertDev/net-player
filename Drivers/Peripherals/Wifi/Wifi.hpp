#ifndef PERIPHERALS_WIFI_WIFI_HPP_
#define PERIPHERALS_WIFI_WIFI_HPP_

#include "WifiSPI.h"
#include "WifiIOSettings.hpp"
#include "Utils.hpp"
#include "Socket.hpp"

#include <vector>

namespace wifi
{
	class Wifi {
	public:
		Wifi(WifiIOSettings io_settings);
		~Wifi();

		std::vector<AP> scan();

		bool set_dhcp_client(bool state);
		bool connect(const char* ssid, const char* password, SECURITY security);
		bool disconnect();

		Socket* open(size_t id, SOCKET_TYPE type, const char* address, uint32_t port);

		size_t ping(const char* address, size_t count);
		const char* get_ip(const char* domain_name);
		uint32_t getUTCTime();
	private:
		friend class Socket;
		bool check_response_ok();
		detail::WifiSPI spi_;
	};
}
#endif /* PERIPHERALS_WIFI_WIFI_HPP_ */
