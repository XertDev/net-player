#include <stdio.h>
#include <string.h>
#include <TouchDebugView/TouchDebugView.hpp>
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"

extern bool volatile detected_touch;

static void draw_background(LCDDisplay& display);
static void draw_y(LCDDisplay& display, int y);

constexpr uint8_t target_backlight_level = 100;

void touchDebugView(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);

	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	// TODO: Handle acquiring a new letter from the remote controller
	auto& touch_panel = pack.touch_panel;
	while(true) {

		while(detected_touch) {

			if(touch_panel.detectTouch() == 1) {
				auto touch_details = touch_panel.getDetails(0);
				if(touch_details.event_type == 1) {
					auto touch_info = touch_panel.getPoint(0);
					draw_y(pack.lcd_display, touch_info.y);
				}
			}
		}
	}
}

static void draw_background(LCDDisplay& display) {
	display.clear(background_color_dark);
	display.setBackgroundColor(background_color_dark);
	display.setTextColor(text_color_white);

	for(int i = 0; i < 8; ++i) {
		display.drawHLine(0, i*30, 240, button_color_green);
		int y = i*30;
		char str[4];
		sprintf(str, "%d", y);
		display.drawString(0, y, str);
	}
}

static void draw_y(LCDDisplay& display, int y) {
	char str[4];
	sprintf(str, "%d", y);
	display.fillRect(100, 100, 60, 30, background_color_dark);
	display.drawString(100, 100, str);
}
