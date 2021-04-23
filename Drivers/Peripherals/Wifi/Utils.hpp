#ifndef PERIPHERALS_WIFI_UTILS_HPP_
#define PERIPHERALS_WIFI_UTILS_HPP_

#include <cstdint>

namespace wifi
{
	enum class SECURITY: uint8_t
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
