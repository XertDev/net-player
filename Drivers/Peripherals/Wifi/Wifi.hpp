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
	private:
		detail::WifiSPI spi_;
	};
}
#endif /* PERIPHERALS_WIFI_WIFI_HPP_ */
