#ifndef PERIPHERALS_WIFI_WIFI_HPP_
#define PERIPHERALS_WIFI_WIFI_HPP_

#include "WifiSPI.h"
#include "WifiIOSettings.hpp"
#include "Utils.hpp"

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

		bool open(size_t id, SOCKET_TYPE type, const char* address, uint32_t port);
		bool close(size_t id);
	private:

		bool check_response_ok();
		detail::WifiSPI spi_;
	};
}
#endif /* PERIPHERALS_WIFI_WIFI_HPP_ */
