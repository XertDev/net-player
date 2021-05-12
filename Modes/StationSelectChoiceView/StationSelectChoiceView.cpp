#include "StationSelectChoiceView.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"

extern bool detected_touch;

constexpr uint8_t target_backlight_level = 100;

static void draw_background(LCDDisplay& display);
static void draw_fav_list_button(LCDDisplay& lcd_display, wifi::Wifi& wifi);
static void draw_wifi_panel_button(LCDDisplay& lcd_display);
static void draw_wifi_info(LCDDisplay& display, wifi::Wifi& wifi);

void stationSelectChoiceView(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	draw_fav_list_button(pack.lcd_display, pack.wifi);
	draw_wifi_panel_button(pack.lcd_display);
	draw_wifi_info(pack.lcd_display, pack.wifi);

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
					if (pack.wifi.is_connected() && inRange(touch_info.x, 20, 110) && inRange(touch_info.y, 20, 110)) {
						uint8_t *last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 2;
						should_change_view = true;
					} else if (inRange(touch_info.x, 130, 220) && inRange(touch_info.y, 20, 110)) {
						uint8_t *last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 4;
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

static void draw_fav_list_button(LCDDisplay& display, wifi::Wifi& wifi) {
	if (wifi.is_connected()) {
		display.fillRect(20, 20, 90, 90, button_color_darkblue);
		display.setBackgroundColor(button_color_darkblue);
	} else {
		display.fillRect(20, 20, 90, 90, background_color_grey);
		display.setBackgroundColor(background_color_grey);
	}
	display.drawString(31, 24, "From");
	display.drawString(39, 53, "Fav");
	display.drawString(31, 82, "List");
}

static void draw_wifi_panel_button(LCDDisplay& display) {
	display.fillRect(130, 20, 90, 90, button_color_darkcyan);
	display.setBackgroundColor(button_color_darkcyan);
	display.drawString(141, 34, "WiFi");
	display.drawString(132, 72, "Panel");
}
static void draw_wifi_info(LCDDisplay& display, wifi::Wifi& wifi) {
	if(wifi.is_connected()){
		display.fillRect(15, 130, 210, 90, button_color_green);
		display.setBackgroundColor(button_color_green);
		display.drawString(43, 132, "Connected");
		display.drawHLine(15, 157, 210, background_color_grey);
		display.drawString(18, 160, wifi.get_connected_name());
	} else {
		display.fillRect(15, 130, 210, 90, button_color_red);
		display.setBackgroundColor(button_color_red);
		display.drawString(18, 161, "Disconnected");
	}
}
