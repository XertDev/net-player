#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <math.h>
#include "RadioView.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"
#include "StationInfo.hpp"


#include "SpiritDSP_MP3_Dec/inc/spiritMP3Dec.h"

extern I2S_HandleTypeDef hi2s2;

extern bool detected_touch;
extern StationInfo current_station;

static void draw_background(LCDDisplay& display);
static void draw_station_name(LCDDisplay& display, const char* station_name);
static void draw_music_info(LCDDisplay& display, const char* music_info, uint8_t offset);
static void draw_volume_info(LCDDisplay& display);
static void update_volume_info(LCDDisplay& display, audio::AudioCodec& codec);
static void draw_station_change_button(LCDDisplay& display);

constexpr uint8_t target_backlight_level = 100;

constexpr uint16_t BUFFER_SIZE = 576;
static int16_t audio_data[2 * BUFFER_SIZE];

TSpiritMP3Decoder g_MP3Decoder;

constexpr uint8_t text_delay = 5;

unsigned int RetrieveMP3Data(void * pMP3CompressedData, unsigned int nMP3DataSizeInChars, void * token) {
	size_t mp3_len;
	((wifi::Socket*) token)->read(pMP3CompressedData, nMP3DataSizeInChars, mp3_len);
	return mp3_len;
}

bool transfer_enabled = false;
bool first_half_ended = false;
bool second_half_ended = true;

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	if(transfer_enabled) {
		first_half_ended = true;
	}
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
	if(transfer_enabled) {
		second_half_ended = true;
	}
}

void radioView(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);

	draw_station_name(pack.lcd_display, current_station.label);
	draw_music_info(pack.lcd_display, "startup-info", 0);
	draw_volume_info(pack.lcd_display);
	update_volume_info(pack.lcd_display, pack.codec);
	draw_station_change_button(pack.lcd_display);

	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	// TODO: Handle volume change and display changed volume

	char* music_info = (char*) malloc(10);
	uint8_t info_offset = 0;
	uint8_t info_move_delay_ticks = text_delay;
	size_t info_len = strlen(music_info);

	const char* ip = pack.wifi.get_ip(current_station.domain);
	wifi::Socket* mp3_socket = pack.wifi.open(0, wifi::SOCKET_TYPE::TCP, ip, current_station.port);

	// stream.rcs.revma.com/an1ugyygzk8uv
	char* res = (char*) malloc(1460);
	char* mp3_info;
	size_t mp3_len;
	char mp3_request[256];
	sprintf(mp3_request, "GET %s HTTP/1.0\r\nHost: %s:%u\r\n\r\n", current_station.subdomain, current_station.domain, current_station.port);

	char* new_music_info = "xdddd";
	char music_info_request[256];
	sprintf(music_info_request, "GET /currentsong HTTP/1.0\r\nHost: %s:%u\r\n\r\n", current_station.domain, current_station.port);
	if(mp3_socket->send(mp3_request, strlen(mp3_request))) {
		mp3_socket->read((void*) res, 1460, mp3_len);
		free(res);
	}

	// Decoding

	SpiritMP3DecoderInit(&g_MP3Decoder, RetrieveMP3Data, NULL, (void*) mp3_socket);
	transfer_enabled = true;
	uint32_t nSamples;

	/* Main Loop */
	bool should_change_view = false;
	auto& touch_panel = pack.touch_panel;
	SpiritMP3Decode(&g_MP3Decoder, audio_data, BUFFER_SIZE/2, NULL);
	//HAL_I2S_Transmit(&hi2s2, (uint16_t*)audio_data, BUFFER_SIZE, HAL_MAX_DELAY);
	HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)audio_data, BUFFER_SIZE);

	uint8_t counter = 0;
	while(true) {
		if(first_half_ended) {
			SpiritMP3Decode(&g_MP3Decoder, audio_data+BUFFER_SIZE, BUFFER_SIZE/2, NULL);
			first_half_ended = false;
		} else if(second_half_ended) {
			SpiritMP3Decode(&g_MP3Decoder, audio_data, BUFFER_SIZE/2, NULL);
			second_half_ended = false;
		}

		//SpiritMP3Decode(&g_MP3Decoder, audio_data, BUFFER_SIZE, NULL);
		//HAL_I2S_Transmit(&hi2s2, (uint16_t*)audio_data, BUFFER_SIZE, HAL_MAX_DELAY);


		/*if(counter%2 == 0) {
			SpiritMP3Decode(&g_MP3Decoder, audio_data, BUFFER_SIZE/2, NULL);
			HAL_I2S_Transmit(&hi2s2, (uint16_t*)audio_data, BUFFER_SIZE/2, HAL_MAX_DELAY);
		} else {
			SpiritMP3Decode(&g_MP3Decoder, audio_data + BUFFER_SIZE, BUFFER_SIZE/2, NULL);
			HAL_I2S_Transmit(&hi2s2, (uint16_t*)(audio_data + BUFFER_SIZE), BUFFER_SIZE/2, HAL_MAX_DELAY);
		}
		counter = (counter+1)%2;*/

		/*
		wifi::Socket* music_info_socket = pack.wifi.open(1, wifi::SOCKET_TYPE::TCP, ip, current_station.port);
		if(music_info_socket->send(music_info_request, strlen(music_info_request))) {
			size_t new_music_info_len = 0;
			new_music_info = music_info_socket->read(400, new_music_info_len);
			char* temp = strstr(new_music_info, "\r\n") +2;
			while(strncmp(temp, "Content-Length:", 15) != 0) {
				temp = strstr(temp, "\r\n") + 2;
			}
			temp += 15;
			char* end = strstr(temp, "\r\n");
			char* content_length = (char*) malloc(end-temp+1);
			memcpy(content_length, temp, end-temp);
			content_length[end-temp] = 0;
			int length = atoi(content_length);
			free(content_length);
			temp = strstr(temp, "\r\n\r\n")+4;

			memcpy(new_music_info, temp, length);
			new_music_info[length] = 0;
		}
		music_info_socket->close();
		*/

		/*
		if(strcmp(music_info, new_music_info) != 0) {
			// Reset offset so that
			if(strlen(music_info) < strlen(new_music_info)) {
				music_info = (char*) realloc(music_info, strlen(new_music_info) + 1);
			}
			strcpy(music_info, new_music_info);
			info_offset = 0;
			info_move_delay_ticks = text_delay;
			info_len = strlen(music_info);
		} else {
			if(info_len <= 14) {

			} else if(info_move_delay_ticks > 0) {
				--info_move_delay_ticks;
			} else {
				info_offset = (info_offset + 1) % info_len;
				if(info_offset == 0) {
					info_move_delay_ticks = text_delay;
				}
			}
		}
		free(new_music_info);
		*/
		//draw_music_info(pack.lcd_display, music_info, info_offset);

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
			mp3_socket->close();
			free(music_info);
			transfer_enabled = false;
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
