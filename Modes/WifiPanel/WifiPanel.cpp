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
static void draw_wifi_block_info(LCDDisplay& display, uint8_t scroll_index, const std::vector<wifi::AP>& ap_info_vector);
static void draw_scroll_buttons(LCDDisplay& display);
static void draw_scroll(LCDDisplay& display, uint8_t index, uint8_t area_count);

constexpr uint8_t target_backlight_level = 100;
constexpr uint8_t wifis_per_screen = 4;

void wifiPanel(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	draw_back_button(pack.lcd_display);
	draw_refresh_button(pack.lcd_display);
	draw_scroll_buttons(pack.lcd_display);

	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	auto wifis = pack.wifi.scan();
	uint8_t current_scroll_index = 0;
	uint8_t areas_count = std::max((size_t) 1, (wifis.size() - 1) / wifis_per_screen + 1);
	draw_scroll(pack.lcd_display, current_scroll_index, areas_count);

	draw_wifi_block_info(pack.lcd_display, current_scroll_index, wifis);

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
					} else if (inRange(touch_info.x, 100, 240) && inRange(touch_info.y, 40, 100)) {
						// Refresh wifis
						wifis = pack.wifi.scan();
						current_scroll_index = 0;
						areas_count = std::max((size_t) 1, (wifis.size() - 1) / wifis_per_screen + 1);
						draw_scroll(pack.lcd_display, current_scroll_index, areas_count);
						draw_wifi_block_info(pack.lcd_display, current_scroll_index, wifis);
					} else if (inRange(touch_info.x, 219, 240) && inRange(touch_info.y, 32, 53)) {
						if(--current_scroll_index < 0) {
							current_scroll_index = areas_count - 1;
						}
						draw_scroll(pack.lcd_display, current_scroll_index, areas_count);
						draw_wifi_block_info(pack.lcd_display, current_scroll_index, wifis);
					} else if (inRange(touch_info.x, 219, 240) && inRange(touch_info.y, 180, 240)) {
						current_scroll_index = (current_scroll_index + 1) % areas_count;
						draw_scroll(pack.lcd_display, current_scroll_index, areas_count);
						draw_wifi_block_info(pack.lcd_display, current_scroll_index, wifis);
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
	uint8_t relative_index = index % wifis_per_screen;
	if(relative_index % 2 == 0) {
		display.fillRect(0, 32 + 52 * relative_index, 218, 52, button_color_green);
		display.setBackgroundColor(button_color_green);
	} else {
		display.fillRect(0, 32 + 52 * relative_index, 218, 52, button_color_lightgreen);
		display.setBackgroundColor(button_color_lightgreen);
	}
	char substr[13];
	int chars_to_display = std::min(strlen(ap_info.ssid), (size_t) 12);
	strncpy(substr, ap_info.ssid, chars_to_display);
	substr[chars_to_display] = '\0';
	display.drawString(1, 32 + 52 * relative_index + 1, substr);
	display.drawString(1, 32 + 52 * relative_index + 27, wifi::securityToString(ap_info.security));
}
static void draw_empty_wifi_info(LCDDisplay& display, uint8_t index) {
	uint8_t relative_index = index % 4;
	display.fillRect(32 + 52 * relative_index, 0, 219, 52, background_color_dark);
}
static void draw_wifi_block_info(LCDDisplay& display, uint8_t scroll_index, const std::vector<wifi::AP>& ap_info_vector) {
	for(uint8_t i = 0; i < 4; ++i) {
		uint8_t vec_index = scroll_index * wifis_per_screen + i;
		if(i < ap_info_vector.size()) {
			draw_wifi_info(display, i, ap_info_vector[vec_index]);
		} else {
			draw_empty_wifi_info(display, i);
		}
	}
}

static void draw_scroll_buttons(LCDDisplay& display) {
	display.fillRect(219, 32, 21, 21, button_color_grey);
	display.setBackgroundColor(button_color_grey);
	for(int i = 0; i <= 8; ++i) {
		display.drawHLine(229 - i, 38 + i, i*2 + 1, text_color_white);
	}

	display.fillRect(219, 219, 21, 21, button_color_grey);
	display.setBackgroundColor(button_color_grey);
	for(int i = 8; i >= 0; --i) {
		display.drawHLine(229 - i, 240 - 7 - i, i*2 + 1, text_color_white);
	}
}

static void draw_scroll(LCDDisplay& display, uint8_t index, uint8_t area_count) {
	uint8_t scroll_height = 166 / area_count;
	display.fillRect(219, 53, 21, scroll_height * index, background_color_dark);
	display.fillRect(219, 53 + scroll_height * index, 21, scroll_height,  background_color_grey);
	display.fillRect(219, 53 + scroll_height * (index + 1), 21, 166 - scroll_height * (index + 1), background_color_dark);
}
