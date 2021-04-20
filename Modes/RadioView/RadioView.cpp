#include <stdio.h>
#include <math.h>
#include "RadioView.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"

extern I2S_HandleTypeDef hi2s2;

extern bool detected_touch;

static void draw_background(LCDDisplay& display);
static void draw_station_name(LCDDisplay& display);
static void draw_station_info(LCDDisplay& display);
static void draw_volume_info(LCDDisplay& display, uint8_t offset);
static void update_volume_info(LCDDisplay& display, audio::AudioCodec& codec);
static void draw_station_change_button(LCDDisplay& display);

constexpr uint8_t target_backlight_level = 100;

constexpr uint16_t BUFFER_SIZE = 2048;
static int16_t audio_data[2 * BUFFER_SIZE];
static void play_tone(void);

void radioView(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	for(uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	draw_station_name(pack.lcd_display);
	draw_station_info(pack.lcd_display);
	draw_volume_info(pack.lcd_display, 0);
	update_volume_info(pack.lcd_display, pack.codec);
	draw_station_change_button(pack.lcd_display);
	// TODO: Handle volume change and display changed volume

	// TODO: Handle extracting current station info and displaying it on the screen

	//play_tone();


	/* Main Loop */
	bool should_change_view = false;
	auto& touch_panel = pack.touch_panel;
	while(true) {

		while(detected_touch) {

			if(touch_panel.detectTouch() == 1) {
				auto touch_details = touch_panel.getDetails(0);
				if(touch_details.event_type == 1) {
					auto touch_info = touch_panel.getPoint(0);
					if(inRange(touch_info.x, 190, 230) && inRange(touch_info.y, 0, 240)) {
						uint8_t* last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 2;
						should_change_view = true;
					}
				}
			}
		}

		if(should_change_view) {
			break;
		}
	}
}

static void draw_background(LCDDisplay& display) {
	display.clear(background_color_dark);
	display.setBackgroundColor(background_color_dark);
	display.setTextColor(text_color_white);
}

static void draw_station_name(LCDDisplay& display) {
	display.setBackgroundColor(background_color_dark);
	// TODO: Consider aligning station name horizontally
	display.drawString(8, 8, "RMF");
	display.drawHLine(0, 40, 240, line_color_lightgrey);
}

static void draw_station_info(LCDDisplay& display) {
	display.setBackgroundColor(background_color_dark);
	// TODO: Consider aligning station name horizontally
	display.drawString(8, 48, "Some Song - Some artists");
	display.drawHLine(0, 40, 240, line_color_lightgrey);
}

static void draw_volume_info(LCDDisplay& display, uint8_t offset) {
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
