#include "FavStationList.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"

constexpr uint8_t target_backlight_level = 100;


static void draw_background(LCDDisplay& display);

void favStationList(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);


	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	while(true) {

	}
}


static void draw_background(LCDDisplay& display) {
	display.clear(background_color_dark);
	display.setBackgroundColor(background_color_dark);
	display.setTextColor(text_color_white);
}
