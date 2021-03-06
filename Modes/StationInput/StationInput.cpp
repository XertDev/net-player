#include <stdio.h>
#include <string.h>
#include "StationInput.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"

extern bool volatile detected_touch;

static void draw_background(LCDDisplay& display);
static void draw_input(LCDDisplay& display, const char* current_input);
static void draw_confirm_button(LCDDisplay& display);

constexpr uint8_t target_backlight_level = 100;

void stationInput(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	draw_confirm_button(pack.lcd_display);

	draw_input(pack.lcd_display, "www.radioooooooooo.pl");

	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}


	// TODO: Handle acquiring a new letter from the remote controller
	bool should_change_view = false;
	auto& touch_panel = pack.touch_panel;
	while(true) {

		while(detected_touch) {

			if(touch_panel.detectTouch() == 1) {
				auto touch_details = touch_panel.getDetails(0);
				if(touch_details.event_type == 1) {
					auto touch_info = touch_panel.getPoint(0);
					if(inRange(touch_info.x, 0, 240) && inRange(touch_info.y, 190, 230)) {
						uint8_t* last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 5;
						should_change_view = true;
					}
				}
			}
		}

		if(should_change_view) {
			break;
		}
	}
}

static void draw_background(LCDDisplay& display) {
	display.clear(background_color_dark);
	display.setBackgroundColor(background_color_dark);
	display.setTextColor(text_color_white);
}

static void draw_input(LCDDisplay& display, const char* current_input) {
	display.setBackgroundColor(background_color_grey);
	char substr[14];
	for(size_t i = 0; i <= strlen(current_input)/14; ++i) {
		display.fillRect(0, 80 + i * 30, 240, 30, background_color_grey);
		strncpy(substr, current_input + 14 * i, 14);
		display.drawString(2, 84 + 30 * i, substr);
	}
}

static void draw_confirm_button(LCDDisplay& display) {
	display.fillRect(0, 190, 240, 40, button_color_green);
	display.setBackgroundColor(button_color_green);
	display.drawString(8, 198, "CONFIRM");
}
