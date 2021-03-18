#ifndef LCD_LCDDISPLAY_HPP_
#define LCD_LCDDISPLAY_HPP_

#include "Common.hpp"
#include "LCDIOSettings.hpp"
#include "LCDConstants.hpp"
#include "stm32f4xx_hal.h"

enum LCDError {
	LCD_OK=0,
	LCD_NOT_FOUND
};

class LCDDisplay {
public:
	LCDDisplay(LCDIOSettings io_settings);
	~LCDDisplay();
	LCDError init();


	void setBacklight(uint8_t level);
	uint8_t backlight();


	void displayOn();
	void displayOff();

	void clear(Color color);

	constexpr uint16_t width();
	constexpr uint16_t height();

	void setCursor(uint16_t x, uint16_t y);

	void drawPixel(uint16_t x, uint16_t y, Color color);

	void drawHLine(uint16_t x, uint16_t y, uint16_t length, Color color);
	void drawVLine(uint16_t x, uint16_t y, uint16_t length, Color color);

	void drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color color);
	void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color color);

	void drawChar(uint16_t x, uint16_t y, char sym);
	void drawString(uint16_t x, uint16_t y, const char* text);

	void setBackgroundColor(Color color);
	void setTextColor(Color color);

	void setOrientation(ST7789H2::ORIENTATION orientation);

private:
	LCDIOSettings io_settings_;

	uint8_t level_{0};

	LCDDRawProperties properties_;

	void resetLCD();
	void initLCD();

	void writeReg(uint8_t reg);
	void writeRegData(uint8_t reg, uint8_t* data, uint8_t count);
	void writeData(uint16_t data);
	uint16_t readData();

	uint16_t readReg(uint8_t reg);

	uint16_t id();
};

#endif /* LCD_LCDDISPLAY_HPP_ */
