#include <cstring>
#include <cctype>
#include "Utils.hpp"


const char* security_strings[] = {"open", "wep", "wpa2", "wpa wpa2", "wpa"};

wifi::SECURITY wifi::parseSecurity(char *sec_str) {
	for(int i = 0; sec_str[i]; i++){
	  sec_str[i] = tolower(sec_str[i]);
	}
	for (uint8_t i = 0; i < sizeof(security_strings) / sizeof(char*); ++i) {
		if (strstr(sec_str, security_strings[i])) {
			return (wifi::SECURITY) i;
		}
	}
	return SECURITY::UNKNOWN;
}
const char* wifi::securityToString(wifi::SECURITY sec) {
	if (sec == wifi::SECURITY::UNKNOWN) {
		return "Unknown";
	} else {
		return security_strings[(uint8_t) sec];
	}
}

