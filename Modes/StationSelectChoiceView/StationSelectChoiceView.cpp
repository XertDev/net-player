#include "StationSelectChoiceView.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"

extern bool detected_touch;

constexpr uint8_t target_backlight_level = 100;

static void draw_background(LCDDisplay& display);
static void draw_fav_list_button(LCDDisplay& lcd_display);
static void draw_input_station_button(LCDDisplay& lcd_display);
static void draw_wifi_panel_button(LCDDisplay& lcd_display);


void stationSelectChoiceView(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	draw_fav_list_button(pack.lcd_display);
	draw_input_station_button(pack.lcd_display);
	draw_wifi_panel_button(pack.lcd_display);

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
					if (inRange(touch_info.x, 20, 110) && inRange(touch_info.y, 20, 110)) {
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
						*last = 3;
						should_change_view = true;
					} else if (inRange(touch_info.x, 20, 110) && inRange(touch_info.y, 130, 220)) {
						uint8_t *last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 4;
						should_change_view = true;
					} else if(inRange(touch_info.x, 130, 220) && inRange(touch_info.y, 130, 220)) {
						/*uint8_t *last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 5;
						should_change_view = true;*/
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

static void draw_fav_list_button(LCDDisplay& display) {
	display.fillRect(20, 20, 90, 90, button_color_green);
	display.setBackgroundColor(button_color_green);
	display.drawString(31, 24, "From");
	display.drawString(39, 53, "Fav");
	display.drawString(31, 82, "List");
}

static void draw_input_station_button(LCDDisplay& display) {
	display.fillRect(130, 20, 90, 90, button_color_darkblue);
	display.setBackgroundColor(button_color_darkblue);
	display.drawString(149, 24, "New");
	display.drawString(141, 53, "From");
	display.drawString(132, 82, "Input");
}

static void draw_wifi_panel_button(LCDDisplay& display) {
	display.fillRect(20, 130, 90, 90, button_color_darkcyan);
	display.setBackgroundColor(button_color_darkcyan);
	display.drawString(31, 144, "WiFi");
	display.drawString(22, 182, "Panel");
}
