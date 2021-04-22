#include <stdio.h>
#include <string.h>
#include "WifiPanel/WifiPanel.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"

extern bool detected_touch;

static void draw_background(LCDDisplay& display);
static void draw_back_button(LCDDisplay& display);
static void draw_refresh_button(LCDDisplay& display);

constexpr uint8_t target_backlight_level = 100;

void wifiPanel(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	draw_back_button(pack.lcd_display);
	draw_refresh_button(pack.lcd_display);

	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	bool should_change_view = false;
	auto& touch_panel = pack.touch_panel;
	while (true) {
		while (detected_touch) {

			if (touch_panel.detectTouch() == 1) {
				auto touch_details = touch_panel.getDetails(0);
				if (touch_details.event_type == 1) {
					auto touch_info = touch_panel.getPoint(0);
					if (inRange(touch_info.x, 5, 115) && inRange(touch_info.y, 5, 115)) {
						uint8_t *last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 1;
						should_change_view = true;
					}
				}
			}
		}

		if (should_change_view) {
			break;
		}
	}
}


static void draw_background(LCDDisplay& display) {
	display.clear(background_color_dark);
	display.setBackgroundColor(background_color_dark);
	display.setTextColor(text_color_white);
}

static void draw_back_button(LCDDisplay& display) {
	display.fillRect(5, 5, 110, 30, button_color_darkblue);
	display.setBackgroundColor(button_color_darkblue);
	display.drawString(8, 8, "BACK");
}

static void draw_refresh_button(LCDDisplay& display) {
	display.fillRect(125, 5, 110, 30, button_color_yellow);
	display.setBackgroundColor(button_color_yellow);
	display.drawString(128, 8, "REFRESH");
}
