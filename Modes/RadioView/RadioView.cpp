#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <math.h>
#include "RadioView.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"

extern I2S_HandleTypeDef hi2s2;

extern bool detected_touch;

static void draw_background(LCDDisplay& display);
static void draw_station_name(LCDDisplay& display, const char* station_name);
static void draw_music_info(LCDDisplay& display, const char* music_info, uint8_t offset);
static void draw_volume_info(LCDDisplay& display);
static void update_volume_info(LCDDisplay& display, audio::AudioCodec& codec);
static void draw_station_change_button(LCDDisplay& display);

constexpr uint8_t target_backlight_level = 100;

constexpr uint16_t BUFFER_SIZE = 2048;
static int16_t audio_data[2 * BUFFER_SIZE];
static void play_tone(void);

void radioView(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);

	draw_station_name(pack.lcd_display, "RMF");
	draw_music_info(pack.lcd_display, "", 0);
	draw_volume_info(pack.lcd_display);
	update_volume_info(pack.lcd_display, pack.codec);
	draw_station_change_button(pack.lcd_display);

	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	// TODO: Handle volume change and display changed volume

	// TODO: Handle extracting current station info and displaying it on the screen

	//play_tone();

	char* music_info = "no info";
	uint8_t info_offset = 0;
	uint8_t info_move_delay_ticks = 10;
	size_t info_len = strlen(music_info);

	const char* ip = pack.wifi.get_ip("stream.rcs.revma.com");
	wifi::Socket* socket = pack.wifi.open(0, wifi::SOCKET_TYPE::TCP, ip, 80);

	// stream.rcs.revma.com/an1ugyygzk8uv
	char* res;
	char* request = "GET /an1ugyygzk8uv HTTP/1.0\r\nHost: stream.rcs.revma.com\r\n\r\n";
	if(socket->send(request, strlen(request))) {
		res = socket->read(1460);
	}

	/* Main Loop */
	bool should_change_view = false;
	auto& touch_panel = pack.touch_panel;
	while(true) {
		HAL_Delay(100);
		// TODO: Get music info
		char* new_music_info = "get info from station";
		if(strcmp(music_info, new_music_info) != 0) {
			// Reset offset so that
			music_info = new_music_info;
			info_offset = 0;
			info_move_delay_ticks = 10;
			info_len = strlen(music_info);
		} else {
			if(info_len <= 14) {

			} else if(info_move_delay_ticks > 0) {
				--info_move_delay_ticks;
			} else {
				info_offset = (info_offset + 1) % info_len;
				if(info_offset == 0) {
					info_move_delay_ticks = 10;
				}
			}
		}
		draw_music_info(pack.lcd_display, music_info, info_offset);

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
						*last = 1;
						should_change_view = true;
					}
				}
			}
		}

		if(should_change_view) {
			socket->close();
			break;
		}
	}
}

static void draw_background(LCDDisplay& display) {
	display.clear(background_color_dark);
	display.setBackgroundColor(background_color_dark);
	display.setTextColor(text_color_white);
}

static void draw_station_name(LCDDisplay& display, const char* station_name) {
	display.setBackgroundColor(background_color_dark);
	// TODO: Consider aligning station name horizontally
	display.drawString(120 - (strlen(station_name)*17)/2, 8, station_name);
	display.drawHLine(0, 40, 240, line_color_lightgrey);
}

static void draw_add_to_fav_button(LCDDisplay& display) {

}

static void clear_music_info_area(LCDDisplay& display, const char* music_info, uint8_t offset) {
	display.setBackgroundColor(background_color_dark);
}
static void draw_music_info(LCDDisplay& display, const char* music_info, uint8_t offset) {
	display.setBackgroundColor(background_color_dark);
	uint8_t char_count = std::min(strlen(music_info) - offset, (size_t)14);
	char substr[14];
	strncpy(substr, music_info + offset, char_count);
	for(int i = char_count; i < 14; ++i) {
		substr[i] = ' ';
	}
	display.drawString(2, 48, substr);
}

static void draw_volume_info(LCDDisplay& display) {
	display.setBackgroundColor(background_color_dark);
	display.drawString(10, 130, "Vol: ");
}

static void update_volume_info(LCDDisplay& display, audio::AudioCodec& codec) {
	char vol_str[4];
	sprintf(vol_str, "%d", codec.getVolume());
	display.setBackgroundColor(background_color_dark);
	display.drawString(78, 130, vol_str);
}

static void draw_station_change_button(LCDDisplay& display) {
	display.fillRect(0, 190, 240, 40, button_color_orange);
	display.setBackgroundColor(button_color_orange);
	display.drawString(8, 198, "Change station");
}

static void play_tone(void) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        int16_t value = (int16_t)(32000.0 * sin(2.0 * M_PI * i / 22.0));
        audio_data[i * 2] = value;
        audio_data[i * 2 + 1] = value;
    }

    while (1)
    {
        HAL_I2S_Transmit(&hi2s2, (uint16_t*)audio_data, 2 * BUFFER_SIZE, HAL_MAX_DELAY);
    }
}
