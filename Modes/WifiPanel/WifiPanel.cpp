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
static void draw_wifi_block_info(LCDDisplay& display, uint8_t starting_index, const std::vector<wifi::AP>& ap_info_vector);

constexpr uint8_t target_backlight_level = 100;

void wifiPanel(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	draw_back_button(pack.lcd_display);
	draw_refresh_button(pack.lcd_display);

	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	auto wifis = pack.wifi.scan();

	draw_wifi_block_info(pack.lcd_display, 0, wifis);

	bool should_change_view = false;
	auto& touch_panel = pack.touch_panel;
	while (true) {
		while (detected_touch) {

			if (touch_panel.detectTouch() == 1) {
				auto touch_details = touch_panel.getDetails(0);
				if (touch_details.event_type == 1) {
					auto touch_info = touch_panel.getPoint(0);
					if (inRange(touch_info.x, 0, 100) && inRange(touch_info.y, 0, 32)) {
						uint8_t *last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 1;
						should_change_view = true;
					} else if (inRange(touch_info.x, 100, 240) && inRange(touch_info.y, 0, 32)) {
						wifis = pack.wifi.scan();

						draw_wifi_block_info(pack.lcd_display, 0, wifis);
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
	display.fillRect(0, 0, 100, 32, button_color_darkblue);
	display.setBackgroundColor(button_color_darkblue);
	display.drawString(4, 4, "BACK");
}

static void draw_refresh_button(LCDDisplay& display) {
	display.fillRect(100, 0, 140, 32, button_color_yellow);
	display.setBackgroundColor(button_color_yellow);
	display.drawString(104, 4, "REFRESH");
}


static void draw_wifi_info(LCDDisplay& display, uint8_t index, wifi::AP ap_info) {
	uint8_t relative_index = index % 4;
	if(relative_index % 2 == 0) {
		display.fillRect(0, 32 + 52 * relative_index, 220, 52, button_color_green);
		display.setBackgroundColor(button_color_green);
	} else {
		display.fillRect(0, 32 + 52 * relative_index, 220, 52, button_color_lightgreen);
		display.setBackgroundColor(button_color_lightgreen);
	}
	display.drawString(1, 32 + 52 * relative_index + 1, ap_info.ssid);
	display.drawString(1, 32 + 52 * relative_index + 27, wifi::securityToString(ap_info.security));
}
static void draw_empty_wifi_info(LCDDisplay& display, uint8_t index) {
	uint8_t relative_index = index % 4;
	display.fillRect(32 + 52 * relative_index, 0, 220, 52, background_color_dark);
}
static void draw_wifi_block_info(LCDDisplay& display, uint8_t starting_index, const std::vector<wifi::AP>& ap_info_vector) {
	for(uint8_t i = 0; i < 4; ++i) {
		if(starting_index + i < ap_info_vector.size()) {
			draw_wifi_info(display, starting_index + i, ap_info_vector[starting_index + i]);
		} else {
			draw_empty_wifi_info(display, starting_index + i);
		}
	}
}
