#include <cstring>
#include <cctype>
#include "Utils.hpp"


const char* security_strings[] = {"open", "wep", "wpa", "wpa2", "wpa wpa2"};

wifi::SECURITY wifi::parseSecurity(char *sec_str) {
	if (strstr(sec_str, "Open")) {
		return SECURITY::NONE;
	} else if (strstr(sec_str, "WEP")) {
		return SECURITY::WEP;
	} else if (strstr(sec_str, "WPA2 AES")) {
		return SECURITY::WPA2;
	} else if (strstr(sec_str, "WPA WPA2")) {
		return SECURITY::WPA_WPA2;
	} else if (strstr(sec_str, "WPA2 TKIP")) {
		return SECURITY::UNKNOWN;    // no match in mbed
	} else if (strstr(sec_str, "WPA2")) {
		return SECURITY::WPA2;    // catch any other WPA2 formula
	} else if (strstr(sec_str, "WPA")) {
		return SECURITY::WPA;
	} else {
		return SECURITY::UNKNOWN;
	}
}
const char* wifi::securityToString(wifi::SECURITY sec) {
	if (sec == wifi::SECURITY::UNKNOWN) {
		return "Unknown";
	} else {
		return security_strings[(uint8_t) sec];
	}
}

