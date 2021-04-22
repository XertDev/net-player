#ifndef PERIPHERALS_WIFI_IO_WIFIIOSETTINGS_HPP_
#define PERIPHERALS_WIFI_IO_WIFIIOSETTINGS_HPP_

#include <PinPort.hpp>
#include "stm32f4xx_hal.h"

namespace wifi
{
	struct WifiIOSettings {
		//reset
		PinPort reset;
		//wakeup
		PinPort wakeup;

		//ssn
		PinPort nss;
		//data_ready
		PinPort data_ready;

		//data communication
		SPI_HandleTypeDef& data;
	};
}


#endif /* PERIPHERALS_WIFI_IO_WIFIIOSETTINGS_HPP_ */
