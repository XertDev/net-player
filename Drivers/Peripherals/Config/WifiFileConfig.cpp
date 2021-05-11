#include "WifiFileConfig.hpp"
#include <cstring>

config::WifiFileConfig::WifiFileConfig(FIL& file) {
	APPassword temp_ap;
	char buff[512];
	size_t received = 0;
	f_gets(temp_ap.ssid, 32, &file);


	while(strlen(temp_ap.ssid) != 0) {
		memset(buff, 0, 512);
		f_gets(buff, 512, &file);

		temp_ap.password = std::string(buff);
		ap_passwords_.emplace_back(temp_ap);
		f_gets(temp_ap.ssid, 32, &file);
	}
}

config::WifiFileConfig::~WifiFileConfig() {
	// TODO Auto-generated destructor stub
}

const std::vector<config::WifiFileConfig::APPassword>& config::WifiFileConfig::APPasswords() {
	return ap_passwords_;
}
