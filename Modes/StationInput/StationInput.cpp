#include "StationInput.hpp"
#include "stm32f4xx_hal.h"

constexpr uint8_t target_backlight_level = 100;
constexpr Color background_color = from_r8g8b8(238, 244, 237);

static void draw_background(LCDDisplay& display);


void stationInput(uint8_t* modes_stack, PeripheralsPack& pack)
{
	draw_background(pack.lcd_display);
	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i)
	{
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	// TODO: Handle acquiring a new letter from the remote controller
	while(true) {

	}
}

static void draw_background(LCDDisplay& display) {
	display.clear(background_color);

	//TODO: Do some magic
}

