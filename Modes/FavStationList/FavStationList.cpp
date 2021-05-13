#include "FavStationList.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"
#include "StationInfo.hpp"

#include <cstring>

extern bool detected_touch;
extern StationInfo current_station;

constexpr uint8_t target_backlight_level = 100;


static void draw_background(LCDDisplay& display);
static void draw_scroll_buttons(LCDDisplay& display);
static void draw_scroll(LCDDisplay& display, uint8_t index, uint8_t area_count);
static void draw_station_area_element(LCDDisplay& display, const char* station_label, uint8_t index);
static void draw_station_areas(LCDDisplay& display, uint8_t scroll_index, const std::vector<StationInfo>& stations);


const uint8_t stations_per_screen = 5;

void favStationList(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	draw_scroll_buttons(pack.lcd_display);

	FIL file;
	std::vector<StationInfo> stations;
	pack.storage.openFile("0:/station_list.txt", file);

	char info_buffer[256];
	do {
		f_gets(info_buffer, sizeof(info_buffer), &file);

		StationInfo station{};

		char* label = strtok(info_buffer, ";");
		station.label = (char*) malloc(strlen(label) + 1);
		strcpy(station.label, label);

		char* domain =  strtok(NULL, ";");
		station.domain = (char*) malloc(strlen(domain) + 1);
		strcpy(station.domain, domain);

		char* subdomain = strtok(NULL, ";");
		station.subdomain = (char*) malloc(strlen(subdomain) + 1);
		strcpy(station.subdomain, subdomain);

		station.port = atoi(strtok(NULL, "\n"));
		stations.push_back(station);
	} while(!f_eof(&file));

	pack.storage.closeFile(file);
	/*stations.push_back(StationInfo{"Radio 357", "stream.rcs.revma.com", "/an1ugyygzk8uv?rj-ttl=5&rj-tok=AAABeWC4tFcAhiTkk11B7NQz7w", 80});
	stations.push_back(StationInfo{"Smooth Jazz Florida", "us4.internet-radio.com", "/", 8266});*/

	uint8_t current_scroll_index = 0;
	uint8_t areas_count = std::max((size_t) 1, (stations.size() - 1) / stations_per_screen + 1);

	draw_scroll(pack.lcd_display, current_scroll_index, areas_count);
	draw_station_areas(pack.lcd_display,  current_scroll_index, stations);


	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	// TODO: This is temporary, change whole main loop
	bool should_change_view = false;
	auto& touch_panel = pack.touch_panel;
	while (true) {
		while (detected_touch) {
			if (touch_panel.detectTouch() == 1) {
				auto touch_details = touch_panel.getDetails(0);
				if (touch_details.event_type == 1) {
					auto touch_info = touch_panel.getPoint(0);
					if (inRange(touch_info.x, 0, 218)) {
						uint32_t station_index = current_scroll_index * stations_per_screen + (touch_info.y/(240/stations_per_screen));
						current_station = stations[station_index];

						uint8_t *last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 5;
						should_change_view = true;
					} else if (inRange(touch_info.y, 0, 30)) {
						if (current_scroll_index == 0) {
							current_scroll_index = areas_count - 1;
						} else {
							--current_scroll_index;
						}
						draw_scroll(pack.lcd_display, current_scroll_index, areas_count);
						draw_station_areas(pack.lcd_display, current_scroll_index, stations);
					} else if(inRange(touch_info.y, 210, 240)) {
						current_scroll_index = (current_scroll_index + 1) % areas_count;
						draw_scroll(pack.lcd_display, current_scroll_index, areas_count);
						draw_station_areas(pack.lcd_display, current_scroll_index, stations);
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

static void draw_scroll_buttons(LCDDisplay& display) {
	display.fillRect(219, 0, 21, 21, button_color_grey);
	display.setBackgroundColor(button_color_grey);
	for(int i = 0; i <= 8; ++i) {
		display.drawHLine(229 - i, 6 + i, i*2 + 1, text_color_white);
	}

	display.fillRect(219, 219, 21, 21, button_color_grey);
	display.setBackgroundColor(button_color_grey);
	for(int i = 8; i >= 0; --i) {
		display.drawHLine(229 - i, 240 - 7 - i, i*2 + 1, text_color_white);
	}
}

static void draw_scroll(LCDDisplay& display, uint8_t index, uint8_t area_count) {
	uint8_t scroll_height = 198 / area_count;
	display.fillRect(219, 22, 21, scroll_height * index, background_color_dark);
	display.fillRect(219, 22 + scroll_height * index, 21, scroll_height,  background_color_grey);
	display.fillRect(219, 22 + scroll_height * (index + 1), 21, 198 - scroll_height * (index + 1), background_color_dark);
}

static void draw_station_area_element(LCDDisplay& display, const char* station_label, uint8_t index) {
	uint8_t relative_index = index % stations_per_screen;
	if(relative_index % 2 == 0) {
		display.fillRect(0, 48 * relative_index, 218, 48, item_color_1_d);
		display.setBackgroundColor(item_color_1_d);

	} else {
		display.fillRect(0, 48 * relative_index, 218, 48, item_color_1_l);
		display.setBackgroundColor(item_color_1_l);
	}
	char label_part[12];
	uint8_t chars = std::min((size_t)11, strlen(station_label));
	strncpy(label_part, station_label, chars);
	label_part[chars] = '\0';
	display.drawString(3, 48 * relative_index, label_part);
	chars = std::min((size_t)11, strlen(station_label + chars));
	strncpy(label_part, station_label + 11, chars);
	label_part[chars] = '\0';
	display.drawString(3, 48 * relative_index + 24, label_part);

	display.fillRect(189, 48 * relative_index, 30, 30, button_color_red);
	display.setBackgroundColor(button_color_red);
	display.drawIcon(195, 48 * relative_index + 3, BIN);
}
static void draw_empty_station_area_element(LCDDisplay& display, uint8_t index) {
	uint8_t relative_index = index % stations_per_screen;
	display.fillRect(0, 48*relative_index, 218, 48, background_color_dark);
}
static void draw_station_areas(LCDDisplay& display, uint8_t scroll_index, const std::vector<StationInfo>& stations) {
	for(uint8_t i = 0; i < stations_per_screen; ++i) {
		uint8_t station_index = scroll_index * stations_per_screen + i;
		if(station_index < stations.size()) {
			draw_station_area_element(display, stations[station_index].label, station_index);
		} else {
			draw_empty_station_area_element(display, station_index);
		}
	}
}
