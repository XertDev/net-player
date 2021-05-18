#include <stdio.h>
#include <string.h>
#include "WifiAuthView/WifiAuthView.hpp"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "ColorPalette.hpp"
#include "Utils.hpp"
#include "Wifi/Utils.hpp"

extern bool detected_touch;
extern wifi::AP current_wifi;

static void draw_background(LCDDisplay& display);
static void draw_back_button(LCDDisplay& display);
static void draw_connect_button(LCDDisplay& display);

static void draw_wifi_name(LCDDisplay& display);
static void draw_password_frame(LCDDisplay& display);
static void draw_password(LCDDisplay& display, const char* password);


static void draw_keyboard(LCDDisplay& display);
static void draw_keyboard_letters(LCDDisplay& display);
static void draw_keyboard_numbers(LCDDisplay& display);
static void draw_keyboard_background(LCDDisplay&display);
static void draw_del_button(LCDDisplay& display);
static void draw_capslock_button(LCDDisplay& display, bool capslock_on);

static void draw_connecting_indicator(LCDDisplay& display);
static void draw_connection_failed(LCDDisplay& display);

constexpr uint8_t target_backlight_level = 100;

constexpr uint32_t key_offset_reset_ticks = 1500;


void wifiAuthView(uint8_t* modes_stack, PeripheralsPack& pack) {
	draw_background(pack.lcd_display);
	draw_back_button(pack.lcd_display);
	draw_connect_button(pack.lcd_display);

	draw_wifi_name(pack.lcd_display);

	char* password = (char*) calloc(33, sizeof(char));
	strcpy(password, "Chyba ty");

	draw_password_frame(pack.lcd_display);
	draw_password(pack.lcd_display, password);

	draw_keyboard(pack.lcd_display);

	bool capslock_on = true;

	for (uint8_t i = pack.lcd_display.backlight(); i <= target_backlight_level; ++i) {
		pack.lcd_display.setBacklight(i);
		HAL_Delay(5);
	}

	uint8_t last_clicked_index = 20;
	uint8_t offset = 0;

	bool should_change_view = false;
	auto &touch_panel = pack.touch_panel;

	uint32_t ticks_since_click = key_offset_reset_ticks;

	while (true) {
		HAL_Delay(1);
		while (detected_touch) {
			if (touch_panel.detectTouch() == 1) {
				auto touch_details = touch_panel.getDetails(0);
				if (touch_details.event_type == 1) {
					ticks_since_click = 0;

					auto touch_info = touch_panel.getPoint(0);
					if (inRange(touch_info.x, 0, 100) && inRange(touch_info.y, 0, 32)) {
						uint8_t *last = modes_stack;
						while (*last != 0) {
							++last;
						}
						*last = 4;
						should_change_view = true;
					} else if(inRange(touch_info.x, 100, 240) && inRange(touch_info.y, 0, 32)) {
						draw_connecting_indicator(pack.lcd_display);

						bool connected = pack.wifi.connect(current_wifi.ssid, password, current_wifi.security);

						if(connected) {
							uint8_t *last = modes_stack;
							while (*last != 0) {
								++last;
							}
							*last = 1;
							should_change_view = true;
						} else {
							draw_connection_failed(pack.lcd_display);
							bool should_break = false;
							while (true) {
								while (detected_touch) {
									if (touch_panel.detectTouch() == 1) {
										auto touch_details = touch_panel.getDetails(0);
										if (touch_details.event_type == 1) {
											auto touch_info = touch_panel.getPoint(0);
											if(inRange(touch_info.x, 0, 100) && inRange(touch_info.y, 0, 32)) {
												should_break = true;
												uint8_t *last = modes_stack;
												while (*last != 0) {
													++last;
												}
												*last = 4;
												should_change_view = true;
											} else if (inRange(touch_info.x, 10, 110) && inRange(touch_info.y, 190, 230)) {
												should_break = true;
												uint8_t *last = modes_stack;
												while (*last != 0) {
													++last;
												}
												*last = 1;
												should_change_view = true;
											} else if(inRange(touch_info.x, 130, 230) && inRange(touch_info.y, 190, 230)) {
												should_break = true;
												draw_keyboard(pack.lcd_display);
											}
										}
									}
								}

								if(should_break) {
									break;
								}
							}
						}
					} else if(inRange(touch_info.x, 0, 61) && inRange(touch_info.y, 207, 240)) {
						if(strlen(password) > 0) {
							password[strlen(password) - 1] = 0;
							draw_password(pack.lcd_display, password);
						}
					} else if(inRange(touch_info.x, 181, 240) && inRange(touch_info.y, 207, 240)) {
						capslock_on = !capslock_on;
						draw_capslock_button(pack.lcd_display, capslock_on);
					} else if(inRange(touch_info.x, 0, 240) && inRange(touch_info.y, 102, 206)) {

						if(strlen(password) < 32) {
							uint8_t char_index = ((touch_info.y - 102)/35 * 4 + (touch_info.x-1)/60);

							char c = '\0';

							uint8_t write_index;

							if(char_index == last_clicked_index) {
								offset = (offset + 1)%3;
								write_index = strlen(password) - 1;
							} else {
								write_index = strlen(password);
								last_clicked_index = char_index;
								offset = 0;
							}

							uint8_t ascii_rel_index = char_index*3 + offset;

							if(char_index < 9) {
								if(ascii_rel_index == 26) {
									c = (char) 48;
								} else {
									c = capslock_on ? (char)(ascii_rel_index + 65) : (char)(ascii_rel_index + 97);
								}
							} else {
								ascii_rel_index -= 26;
								c = (char)(ascii_rel_index + 48);
							}

							password[write_index] = c;
							draw_password(pack.lcd_display, password);
						}
					} else if(inRange(touch_info.x, 61, 181) && inRange(touch_info.y, 207, 240)) {
						if(strlen(password) < 32) {
							password[strlen(password)] = ' ';
							draw_password(pack.lcd_display, password);
						}
					}
				}
			}
		}

		if(ticks_since_click < key_offset_reset_ticks) {
			if(++ticks_since_click == key_offset_reset_ticks) {
				last_clicked_index = 20;
				offset = 0;
			}
		}

		if (should_change_view) {
			break;
			free(password);
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
static void draw_connect_button(LCDDisplay& display) {
	display.fillRect(100, 0, 140, 32, button_color_orange);
	display.setBackgroundColor(button_color_orange);
	display.drawString(104, 4, "CONNECT");
}

static void draw_wifi_name(LCDDisplay& display) {
	char substr[15];
	int chars_to_display = std::min(strlen(current_wifi.ssid), (size_t) 14);
	strncpy(substr, current_wifi.ssid, chars_to_display);
	substr[chars_to_display] = '\0';

	display.setBackgroundColor(background_color_dark);
	display.drawString(2, 35, substr);
}
static void draw_password_frame(LCDDisplay& display) {
	display.drawHLine(0, 63, 240, button_color_darkcyan);
	display.drawHLine(0, 64, 240, button_color_darkcyan);
	display.drawHLine(0, 98, 240, button_color_darkcyan);
	display.drawHLine(0, 99, 240, button_color_darkcyan);

	display.drawVLine(0, 65, 33, button_color_darkcyan);
	display.drawVLine(1, 65, 33, button_color_darkcyan);
	display.drawVLine(238, 65, 33, button_color_darkcyan);
	display.drawVLine(239, 65, 33, button_color_darkcyan);
}
static void draw_password(LCDDisplay& display, const char* password) {
	display.fillRect(2, 65, 236, 33, background_color_dark);

	char substr[14];
	int chars_to_display = std::min(strlen(password), (size_t) 13);
	uint8_t offset = 0;
	if(chars_to_display == 13) {
		offset = strlen(password) - chars_to_display;
	}
	strncpy(substr, password + offset, chars_to_display);
	substr[chars_to_display] = '\0';

	display.setBackgroundColor(background_color_dark);
	display.setTextColor(text_color_white);
	display.drawString(4, 71, substr);
	display.drawVLine(6 + 17*chars_to_display, 71, 24, text_color_white);
}

static void draw_keyboard(LCDDisplay& display) {
	display.setBackgroundColor(background_color_white);
	display.setTextColor(text_color_black);

	draw_keyboard_background(display);
	draw_keyboard_letters(display);
	draw_keyboard_numbers(display);

	draw_del_button(display);
	draw_capslock_button(display, true);

	display.setBackgroundColor(background_color_dark);
	display.setTextColor(text_color_white);
}
static void draw_keyboard_background(LCDDisplay& display) {
	display.fillRect(0, 101, 240, 139, background_color_dark);
	for(uint8_t i = 0; i < 4; ++i) {
		for(uint8_t j = 0; j < 4; ++j) {
			display.fillRect(1 + i*60, 102 + j * 35, 59, 34, background_color_white);
		}
	}
	// Space
	display.drawVLine(120, 207, 34, background_color_white);
}
static void draw_keyboard_letters(LCDDisplay& display) {
	for(uint8_t i = 0; i < 26; ++i) {
		uint8_t a = i/3;
		uint8_t b = i%3;
		uint8_t x = a%4;
		uint8_t y = a/4;
		display.drawChar(5 + x*60 + b*17, 108 + y*35, (char)(65 + i));
	}
}
static void draw_keyboard_numbers(LCDDisplay& display) {
	for(uint8_t num = 0; num < 10; ++num) {
		uint8_t i = num+2;
		uint8_t x = i/3;
		uint8_t offset = i%3;
		display.drawChar(5 + x*60 + offset*17, 178, (char)(48 + num));
	}
}

static void draw_del_button(LCDDisplay& display) {
	display.fillRect(1, 207, 59, 34, button_color_red);
	display.setBackgroundColor(button_color_red);
	display.setTextColor(text_color_white);
	display.drawChar(23, 212, 'X');
	display.setBackgroundColor(background_color_white);
	display.setTextColor(text_color_black);
}
static void draw_capslock_button(LCDDisplay& display, bool capslock_on) {
	if(capslock_on) {
		display.fillRect(181, 207, 59, 34, button_color_darkblue);
		display.setTextColor(text_color_white);
		display.setBackgroundColor(button_color_darkblue);
		display.drawIcon(203, 212, SHIFT);
		display.setTextColor(text_color_black);
	} else {
		display.fillRect(181, 207, 59, 34, background_color_white);
		display.setBackgroundColor(background_color_white);
		display.setTextColor(text_color_black);
		display.drawIcon(203, 212, SHIFT);
	}
}

static void draw_connecting_indicator(LCDDisplay& display) {
	display.fillRect(0, 102, 240, 138, background_color_dark);
	display.setBackgroundColor(background_color_dark);
	display.setTextColor(text_color_white);

	display.drawString(2, 150, "Connecting...");
}
static void draw_connection_failed(LCDDisplay& display) {
	display.fillRect(0, 102, 240, 138, background_color_dark);
	display.setBackgroundColor(background_color_dark);
	display.setTextColor(text_color_white);

	display.drawString(43, 110, "Failed to");
	display.drawString(60, 134, "connect");
	display.drawString(35, 158, "Try again?");

	display.fillRect(10, 190, 100, 40, button_color_red);
	display.setBackgroundColor(button_color_red);
	display.drawString(43, 198, "NO");


	display.fillRect(130, 190, 100, 40, button_color_green);
	display.setBackgroundColor(button_color_green);
	display.drawString(155, 198, "YES");
}
