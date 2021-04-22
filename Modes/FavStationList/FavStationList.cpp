#include "FavStationList.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"

constexpr uint8_t target_backlight_level = 100;


static void draw_background(LCDDisplay& display);
static void draw_scroll_buttons(LCDDisplay& display);
static void draw_scroll(LCDDisplay& display);
static void draw_station_area_element(LCDDisplay& display, uint8_t index);

void favStationList(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	draw_scroll_buttons(pack.lcd_display);


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

static void draw_scroll_buttons(LCDDisplay& display) {
	display.fillRect(229, 0, 11, 7, button_color_grey);
	display.setBackgroundColor(button_color_grey);
	for(int i = 0; i < 5; ++i) {
		display.drawHLine(235 - i, 1 + i, i*2 + 1, text_color_white);
	}

	display.fillRect(239, 233, 11, 7, button_color_grey);
	display.setBackgroundColor(button_color_grey);
	for(int i = 4; i >= 0; --i) {
		display.drawHLine(235 - i, 238 - i, i*2 + 1, text_color_white);
	}
}

static void draw_scroll(LCDDisplay& display, uint8_t index, uint8_t area_count) {
	uint8_t scroll_height = 226 / area_count;
	display.fillRect(229, 7, 11, scroll_height * index, background_color_grey);
	display.fillRect(229, 7 + scroll_height * index, 11, scroll_height, background_color_dark);
	display.fillRect(229, 7 + scroll_height * (index + 1), 11, 226 - scroll_height * (index + 1), background_color_grey);
}

static void draw_station_area_element(LCDDisplay& display, const char* station_label, uint8_t index) {
	uint8_t relative_index = index % 8;
	if(relative_index % 2 == 0) {
		display.fillRect(0, 30 * relative_index, 198, 30, button_color_green);
		display.setBackgroundColor(button_color_green);
		display.drawString(3, 30 * relative_index + 3, station_label);

		display.fillRect(199, 30 * relative_index, 30, 30, button_color_red);
		display.setBackgroundColor(button_color_red);
		display.drawIcon(205, 30 * relative_index + 3, BIN);
	} else {
		display.fillRect(0, 30 * relative_index, 197, 30, button_color_lightgreen);
		display.setBackgroundColor(button_color_lightgreen);
		display.drawString(3, 30 * relative_index + 3, station_label);

		display.fillRect(199, 30 * relative_index, 30, 30, button_color_lightred);
		display.setBackgroundColor(button_color_lightred);
		display.drawIcon(205, 30 * relative_index + 3, BIN);
	}
}
