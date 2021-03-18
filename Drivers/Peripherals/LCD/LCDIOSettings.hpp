#ifndef LCD_LCDIOSETTINGS_HPP_
#define LCD_LCDIOSETTINGS_HPP_

	#include "../PinPort.hpp"
	#include "stm32f4xx_hal.h"
	#include <stdint.h>

	struct LCDIOSettings {
		//backlight
		TIM_HandleTypeDef& backlight_pwm;
		uint32_t backlight_channel;

		//#lcd
		//tearing
		PinPort tearing;
		//reset
		PinPort reset;
		//bank_address
		LCDController* lcd_bank;
	};



#endif /* LCD_LCDIOSETTINGS_HPP_ */
