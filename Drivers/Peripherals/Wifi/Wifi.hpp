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
		bool is_connected();
		const char* get_connected_name();
	private:
		int8_t current_socket = -1;
		uint16_t packet_size = 0;
		uint16_t packet_timeout = 500;

		friend class Socket;
		bool check_response_ok();
		bool check_response_ok_fast();
		detail::WifiSPI spi_;
		bool connected_state_ = false;
		char* connected_name_ = "";
	};
}
#endif /* PERIPHERALS_WIFI_WIFI_HPP_ */
