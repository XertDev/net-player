#ifndef PERIPHERALS_WIFI_UTILS_HPP_
#define PERIPHERALS_WIFI_UTILS_HPP_

#include <cstdint>

namespace wifi
{
	enum class SOCKET_TYPE: uint8_t
	{
		TCP,
		UDP,
		UDP_LITE,
		TCP_SSL,
		MQTT
	};

	enum class SECURITY: uint8_t // TODO: fix order
	{
		NONE,
		WEP,
		WPA2,
		WPA_WPA2,
		WPA,
		UNKNOWN
	};


	SECURITY parseSecurity(char* sec_str);
	const char* securityToString(SECURITY sec);

	struct AP
	{
		char ssid[32+1];
		uint8_t bssid[6];
		SECURITY security;
		int8_t rssi;
		uint8_t channel;
	};
}




#endif /* PERIPHERALS_WIFI_UTILS_HPP_ */
