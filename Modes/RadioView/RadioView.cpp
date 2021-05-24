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

extern bool volatile detected_touch;
extern StationInfo current_station;

static void draw_background(LCDDisplay& display);
static void draw_station_name(LCDDisplay& display, const char* station_name);
static void draw_music_info(LCDDisplay& display, const char* music_info, uint8_t offset);
static void draw_volume_info(LCDDisplay& display);
static void update_volume_info(LCDDisplay& display, audio::AudioCodec& codec);
static void draw_station_change_button(LCDDisplay& display);

static void draw_vol_down_button(LCDDisplay& display);
static void draw_vol_up_button(LCDDisplay& display);
static void draw_vol_div_button(LCDDisplay& display);
static void draw_vol_mult_button(LCDDisplay& display);

constexpr uint8_t target_backlight_level = 100;

constexpr uint16_t BUFFER_SIZE = 1152;//576;

FIL testFile;
int16_t sound[2 * BUFFER_SIZE];
unsigned int br;

TSpiritMP3Decoder g_MP3Decoder;

constexpr uint8_t text_delay = 5;

unsigned int RetrieveMP3Data(void * pMP3CompressedData, unsigned int nMP3DataSizeInChars, void * token) {
	size_t mp3_len;
	((wifi::Socket*) token)->read(pMP3CompressedData, nMP3DataSizeInChars, mp3_len);
	return mp3_len;
}
unsigned int RetrieveMP3DataTest(void * pMP3CompressedData, unsigned int nMP3DataSizeInChars, void * token) {
	f_read(&testFile, pMP3CompressedData, nMP3DataSizeInChars, &br);
	return br;
}

bool transfer_enabled = false;
bool first_half_ended = false;
bool second_half_ended = true;

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	if(transfer_enabled) {
		SpiritMP3Decode(&g_MP3Decoder, sound, BUFFER_SIZE/2, NULL);
	}
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
	if(transfer_enabled) {
		SpiritMP3Decode(&g_MP3Decoder, sound+BUFFER_SIZE, BUFFER_SIZE/2, NULL);
	}
}

void radioView(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);

	draw_station_name(pack.lcd_display, current_station.label);
	draw_music_info(pack.lcd_display, "startup-info", 0);
	draw_station_change_button(pack.lcd_display);
	draw_volume_info(pack.lcd_display);
	update_volume_info(pack.lcd_display, pack.codec);

	draw_vol_down_button(pack.lcd_display);
	draw_vol_up_button(pack.lcd_display);
	draw_vol_div_button(pack.lcd_display);
	draw_vol_mult_button(pack.lcd_display);

	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	// TODO: Handle volume change and display changed volume

	char* music_info = (char*) calloc(400,0);
	uint8_t info_offset = 0;
	uint8_t info_move_delay_ticks = text_delay;
	size_t info_len = strlen(music_info);

	const char* ip = pack.wifi.get_ip(current_station.domain);
	wifi::Socket* mp3_socket = pack.wifi.open(0, wifi::SOCKET_TYPE::TCP, ip, current_station.port);

	char res[1460];
	char* mp3_info;
	size_t mp3_len;
	char mp3_request[256];
	sprintf(mp3_request, "GET %s HTTP/1.0\r\nHost: %s:%u\r\n\r\n", current_station.subdomain, current_station.domain, current_station.port);

	char* new_music_info = (char*) calloc(400, sizeof(char));
	char music_info_request[256];
	sprintf(music_info_request, "GET /currentsong HTTP/1.0\r\nHost: %s:%u\r\n\r\n", current_station.domain, current_station.port);
	if(mp3_socket->send(mp3_request, strlen(mp3_request))) {
		mp3_socket->read((void*) res, 1460, mp3_len);
	}

	// Decoding


	/* Main Loop */

	bool should_change_view = false;
	auto& touch_panel = pack.touch_panel;

	//pack.storage.openFile("music.mp3", testFile);

	SpiritMP3DecoderInit(&g_MP3Decoder, RetrieveMP3Data, NULL, (void*) mp3_socket);

	TSpiritMP3Info info;
	SpiritMP3Decode(&g_MP3Decoder, sound, BUFFER_SIZE, &info);

	// Does not work with this :c

	HAL_I2S_DeInit(&hi2s2);
	hi2s2.Init.AudioFreq =  audio::parseFreqSys(info.nSampleRateHz);
	HAL_I2S_Init(&hi2s2);
	pack.codec.init(audio::OUTPUT_DEVICE::HEADPHONE, audio::parseFreq(info.nSampleRateHz));

	transfer_enabled = true;


	HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)sound, BUFFER_SIZE*2);

	while(true) {

		/*
		wifi::Socket* music_info_socket = pack.wifi.open(1, wifi::SOCKET_TYPE::TCP, ip, current_station.port);
		if(music_info_socket->send(music_info_request, strlen(music_info_request))) {
			size_t new_music_info_len = 0;
			music_info_socket->read(new_music_info, 400, new_music_info_len);
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

		draw_music_info(pack.lcd_display, music_info, info_offset);
		 */


		while(detected_touch) {

			if(touch_panel.detectTouch() == 1) {
				auto touch_details = touch_panel.getDetails(0);
				if(touch_details.event_type == 1) {
					auto touch_info = touch_panel.getPoint(0);
					if(inRange(touch_info.x, 0, 240) && inRange(touch_info.y, 200, 240)) {
						uint8_t* last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 1;
						should_change_view = true;
					} else if(inRange(touch_info.x, 140, 180) && inRange(touch_info.y, 100, 140)) {
						pack.codec.setVolume(pack.codec.getVolume() - 1);
						update_volume_info(pack.lcd_display, pack.codec);
					} else if(inRange(touch_info.x, 190, 230) && inRange(touch_info.y, 100, 140)) {
						pack.codec.setVolume(pack.codec.getVolume() + 1);
						update_volume_info(pack.lcd_display, pack.codec);
					} else if(inRange(touch_info.x, 140, 180) && inRange(touch_info.y, 150, 190)) {
						pack.codec.setVolume(pack.codec.getVolume() / 2);
						update_volume_info(pack.lcd_display, pack.codec);
					} else if(inRange(touch_info.x, 190, 230) && inRange(touch_info.y, 150, 190)) {
						pack.codec.setVolume(pack.codec.getVolume() * 2);
						update_volume_info(pack.lcd_display, pack.codec);
					}
				}
			}
		}

		if(should_change_view) {
			HAL_I2S_DMAStop(&hi2s2);
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
	for(int i = char_count; i < 13; ++i) {
		substr[i] = ' ';
	}
	substr[13] = '\0';
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
	display.fillRect(78, 130, 40, 24, background_color_dark);
	display.drawString(78, 130, vol_str);
}

static void draw_station_change_button(LCDDisplay& display) {
	display.fillRect(0, 200, 240, 40, button_color_orange);
	display.setBackgroundColor(button_color_orange);
	display.drawString(1, 208, "Change station");
}

static void draw_vol_down_button(LCDDisplay& display) {
	display.fillRect(140, 100, 40, 40, button_color_red);
	display.setBackgroundColor(button_color_red);
	display.drawChar(152, 108, '-');
}
static void draw_vol_up_button(LCDDisplay& display) {
	display.fillRect(190, 100, 40, 40, button_color_green);
	display.setBackgroundColor(button_color_green);
	display.drawChar(202, 108, '+');
}
static void draw_vol_div_button(LCDDisplay& display) {
	display.fillRect(140, 150, 40, 40, button_color_red);
	display.setBackgroundColor(button_color_red);
	display.drawString(143, 158, "/2");
}
static void draw_vol_mult_button(LCDDisplay& display) {
	display.fillRect(190, 150, 40, 40, button_color_green);
	display.setBackgroundColor(button_color_green);
	display.drawString(193, 158, "x2");
}
