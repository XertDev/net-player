#include <stdio.h>
#include <math.h>
#include "RadioView.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"

extern I2S_HandleTypeDef hi2s2;

constexpr Color station_info_color = from_r8g8b8(238, 244, 237);
constexpr Color volume_color = from_r8g8b8(238, 244, 237);

constexpr Color background_color = from_r8g8b8(238, 244, 237);
constexpr uint8_t target_backlight_level = 100;

static void draw_background(LCDDisplay& display);
static void draw_station_info(LCDDisplay& display);
static void draw_volume_info(LCDDisplay& display);
static void update_volume_info(LCDDisplay& display, audio::AudioCodec& codec);

constexpr uint16_t BUFFER_SIZE = 2048;
static int16_t audio_data[2 * BUFFER_SIZE];
static void play_tone(void);

void radioView(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i)
	{
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	draw_station_info(pack.lcd_display);
	draw_volume_info(pack.lcd_display);
	update_volume_info(pack.lcd_display, pack.codec);
	// TODO: Handle volume change and display changed volume

	// TODO: Handle extracting current station info and displaying it on the screen



	play_tone();
}

static void draw_background(LCDDisplay& display) {
	display.clear(background_color);

	//TODO: Do some magic
}

static void draw_station_info(LCDDisplay& display) {
	display.setBackgroundColor(station_info_color);
	display.drawString(10, 10, "RMF");
}

static void draw_volume_info(LCDDisplay& display) {
	display.setBackgroundColor(volume_color);
	display.drawString(10, 130, "Vol: ");
}

static void update_volume_info(LCDDisplay& display, audio::AudioCodec& codec) {
	display.setBackgroundColor(volume_color);
	char vol_str[4];
	sprintf(vol_str, "%d", codec.getVolume());
	display.drawString(130, 130, vol_str);
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
