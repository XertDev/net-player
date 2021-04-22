#ifndef PERIPHERALS_WIFI_UTILS_HPP_
#define PERIPHERALS_WIFI_UTILS_HPP_

namespace wifi
{
	enum class SECURITY: uint8_t
	{
		NONE,
		WEP,
		WPA,
		WPA2,
		WPA_WPA2,
		PAP,
		CHAP,
		EAP_TLS,
		PEAP,
		WPA2_ENT,
		WPA3,
		WPA3_WPA2,
		UNKNOWN
	};

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
