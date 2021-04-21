#ifndef LCD_Common_HPP_
#define LCD_Common_HPP_

#include <stdint.h>

struct LCDController {
	volatile uint16_t reg;
	volatile uint16_t ram;
};

typedef uint32_t Color;

struct Font {
	const uint8_t *table;
	uint16_t width;
	uint16_t height;
};

struct LCDDRawProperties {
	Color text_color;
	Color background_color;
	Font* font;
	Font* icon_font;
};

struct Point {
	int16_t x;
	int16_t y;
};

enum Icons{
	BIN = 0,
	HEART = 1
};

constexpr Color from_r8g8b8(uint8_t r, uint8_t g, uint8_t b) {
	uint8_t red = r/8;
	uint8_t green = g/4;
	uint8_t blue = b/8;
	return (red & 0x1f) << 11 | (green & 0x3f) << 5 | (blue & 0x1f);
}
#endif /* LCD_COMMON_HPP_ */
