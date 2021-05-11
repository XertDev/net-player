#include <stdio.h>
#include <string>
#include <vector>
#include <ff.h>

#ifndef CONFIG_WIFIFILECONFIG_HPP_
#define CONFIG_WIFIFILECONFIG_HPP_

namespace config {
	class WifiFileConfig {
	public:
		struct APPassword {
			char ssid[32+1];
			std::string password;
		};

		WifiFileConfig(FIL& file);
		~WifiFileConfig();

		const std::vector<APPassword>& APPasswords();

	private:
		std::vector<APPassword> ap_passwords_;
	};
}

#endif /* CONFIG_WIFIFILECONFIG_HPP_ */
