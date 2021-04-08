//Based on BSP driver

#include "LCDDisplay.hpp"
#include "LCDConstants.hpp"
#include "Common.hpp"
#include "fonts.hpp"
#include "stdint.h"

LCDDisplay::LCDDisplay(
		LCDIOSettings io_settings
		)
:io_settings_(io_settings)
{
	initLCD();
}

LCDDisplay::~LCDDisplay() {
	HAL_TIM_PWM_Stop(&io_settings_.backlight_pwm, io_settings_.backlight_channel);
}

void LCDDisplay::setBacklight(uint8_t level) {
	assert_param(level <= 100);

	const auto& period = io_settings_.backlight_pwm.Init.Period;

	TIM_OC_InitTypeDef config;

	config.OCMode = TIM_OCMODE_PWM1;
	config.Pulse = level/100.0f * period;
	config.OCPolarity = TIM_OCPOLARITY_HIGH;
	config.OCFastMode = TIM_OCFAST_DISABLE;

	HAL_TIM_PWM_ConfigChannel(&io_settings_.backlight_pwm, &config, io_settings_.backlight_channel);
	HAL_TIM_PWM_Start(&io_settings_.backlight_pwm, io_settings_.backlight_channel);

	level_ = level;
}

LCDError LCDDisplay::init() {
	properties_.background_color = 0xFFFF;
	properties_.text_color = 0x0000;
	properties_.font = &Font24;

	resetLCD();

	if(id() != ST7789H2::ID) {
		return LCD_NOT_FOUND;
	}

	initLCD();
	return LCD_OK;
}

void LCDDisplay::resetLCD() {
	//hardware reset
	HAL_GPIO_WritePin(io_settings_.reset.GPIOx, io_settings_.reset.GPIO_Pin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(io_settings_.reset.GPIOx, io_settings_.reset.GPIO_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(io_settings_.reset.GPIOx, io_settings_.reset.GPIO_Pin, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(io_settings_.reset.GPIOx, io_settings_.reset.GPIO_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
}

void LCDDisplay::writeReg(uint8_t reg) {
	io_settings_.lcd_bank->reg = reg;
	__DSB();
}

void LCDDisplay::writeData(uint16_t data) {
	io_settings_.lcd_bank->ram = data;
	__DSB();
}

void LCDDisplay::initLCD() {
	//todo:?
	//sleep in
	writeReg(ST7789H2::SLEEP_IN);
	HAL_Delay(10);

	//software reset
	writeReg(0x01);
	HAL_Delay(200);

	//sleep out
	writeReg(ST7789H2::SLEEP_OUT);
	HAL_Delay(120);

	uint8_t   parameters[14];

	//down side
	parameters[0] = 0x00;
	writeRegData(ST7789H2::NORMAL_DISPLAY, parameters, 1);

	//16bit/pixel
	parameters[0] = 0x05;
	writeRegData(ST7789H2::COLOR_MODE, parameters, 1);

	//display inversion
	writeReg(ST7789H2::DISPLAY_INVERSION);

	//column address caset
	parameters[0] = 0x00;
	parameters[1] = 0x00;
	parameters[2] = 0x00;
	parameters[3] = 0xEF;
	writeRegData(ST7789H2::CASET, parameters, 4);

	//row address raset
	parameters[0] = 0x00;
	parameters[1] = 0x00;
	parameters[2] = 0x00;
	parameters[3] = 0xEF;
	writeRegData(ST7789H2::RASET, parameters, 4);

	//-------Frame rate setting
	//PORCH control setting
	parameters[0] = 0x0C;
	parameters[1] = 0x0C;
	parameters[2] = 0x00;
	parameters[3] = 0x33;
	parameters[4] = 0x33;
	writeRegData(ST7789H2::PORCH_CTRL, parameters, 5);

	//GATE control setting
	parameters[0] = 0x35;
	writeRegData(ST7789H2::GATE_CTRL, parameters, 1);

	//-------Power Settings
	//VCOM setting
	parameters[0] = 0x1F;
	writeRegData(ST7789H2::VCOM_SET, parameters, 1);

	//LCM Control setting
	parameters[0] = 0x2C;
	writeRegData(ST7789H2::LCM_CTRL, parameters, 1);

	//VDV & VRH command enable
	parameters[0] = 0x01;
	parameters[1] = 0xC3;
	writeRegData(ST7789H2::VDV_VRH_EN, parameters, 2);

	//VDV set
	parameters[0] = 0x20;
	writeRegData(ST7789H2::VDV_SET, parameters, 1);

	//Frame rate control in normal mode
	parameters[0] = 0x0F;
	writeRegData(ST7789H2::FR_CTRL,	parameters, 1);

	//Power control
	parameters[0] = 0xA4;
	parameters[1] = 0xA1;
	writeRegData(ST7789H2::POWER_CTRL, parameters, 2);

	//gamma settings
	  parameters[0] = 0xD0;
	  parameters[1] = 0x08;
	  parameters[2] = 0x11;
	  parameters[3] = 0x08;
	  parameters[4] = 0x0C;
	  parameters[5] = 0x15;
	  parameters[6] = 0x39;
	  parameters[7] = 0x33;
	  parameters[8] = 0x50;
	  parameters[9] = 0x36;
	  parameters[10] = 0x13;
	  parameters[11] = 0x14;
	  parameters[12] = 0x29;
	  parameters[13] = 0x2D;
	  writeRegData(ST7789H2::PV_GAMMA_CTRL, parameters, 14);

	  //negative voltage gamma control
	  parameters[0] = 0xD0;
	  parameters[1] = 0x08;
	  parameters[2] = 0x10;
	  parameters[3] = 0x08;
	  parameters[4] = 0x06;
	  parameters[5] = 0x06;
	  parameters[6] = 0x39;
	  parameters[7] = 0x44;
	  parameters[8] = 0x51;
	  parameters[9] = 0x0B;
	  parameters[10] = 0x16;
	  parameters[11] = 0x14;
	  parameters[12] = 0x2F;
	  parameters[13] = 0x31;
	  writeRegData(ST7789H2::NV_GAMMA_CTRL, parameters, 14);

	  displayOn();

	  parameters[0] = 0x00;
	  writeRegData(ST7789H2::TEARING_EFFECT, parameters, 1);
}

void LCDDisplay::writeRegData(uint8_t reg, uint8_t *data, uint8_t count) {
	writeReg(reg);
	for(uint8_t i = 0; i < count; ++i) {
		writeData(data[i]);
	}
}

void LCDDisplay::displayOn() {
	writeReg(ST7789H2::DISPLAY_ON);
	writeReg(ST7789H2::SLEEP_OUT);
}

void LCDDisplay::setOrientation(ST7789H2::ORIENTATION orientation) {
	uint8_t parameter[6];

	if(orientation == ST7789H2::ORIENTATION::LANDSCAPE) {
		parameter[0] = 0x00;
	}
	else if(orientation == ST7789H2::ORIENTATION::LANDSCAPE_ROT180) {
		//vertical scroll
		parameter[0] = 0x00;
		parameter[1] = 0x00;
		//vertical scrolling area
		parameter[2] = 0x01;
		parameter[3] = 0xF0;
		//bottom fixed aread
		parameter[4] = 0x00;
		parameter[5] = 0x00;

		writeRegData(ST7789H2::VSCRDEF, parameter, 6);
		parameter[0] = 0xC0;
		parameter[1] = 0x50;
		writeRegData(ST7789H2::VSCSAD, parameter, 2);

		parameter[0] = 0xC0;
	}
	else {
		parameter[0] = 0x60;
	}

	writeRegData(ST7789H2::NORMAL_DISPLAY, parameter, 1);
}

void LCDDisplay::displayOff() {

	//display off
	writeReg(ST7789H2::DISPLAY_OFF);
	//sleep in
	writeReg(ST7789H2::SLEEP_IN);
	HAL_Delay(10);
}

constexpr uint16_t LCDDisplay::width() {
	return ST7789H2::LCD_WIDTH;
}

constexpr uint16_t LCDDisplay::height() {
	return ST7789H2::LCD_HEIGHT;
}

void LCDDisplay::drawHLine(uint16_t x, uint16_t y, uint16_t length, Color color) {
	setCursor(x, y);
	writeReg(ST7789H2::WRITE_RAM);

	for(uint16_t i = 0; i < length; ++i) {
		writeData(color);
	}
}


void LCDDisplay::drawVLine(uint16_t x, uint16_t y, uint16_t length, Color color) {
	for(uint16_t i = 0; i < length; ++i) {
		drawPixel(x, y+i, color);
	}
}

void LCDDisplay::drawRect(uint16_t x, uint16_t y, uint16_t width,
		uint16_t height, Color color) {
	// +---2---+
	// |       |
	// 1       3
	// |       |
	// +---4---+

	drawVLine(x, y, height, color);   		//1
	drawHLine(x, y, width, color);  		//2
	drawVLine(x+width, y, height, color);	//3
	drawHLine(x, y+height, width, color);	//4
}

void LCDDisplay::fillRect(uint16_t x, uint16_t y, uint16_t width,
		uint16_t height, Color color) {
	for(uint16_t i = 0; i < height; ++i) {
		drawHLine(x, y+i, width, color);
	}
}

void LCDDisplay::drawChar(uint16_t x, uint16_t y, char sym) {
	//based on official draw char from CUBE MX
	const auto& height = properties_.font->height;
	const auto& width = properties_.font->width;
	const uint8_t font_width_bytes = (width+7)/8;

	const uint16_t sym_id = (sym - ' ') * height * font_width_bytes;
	const uint8_t* sym_data = &properties_.font->table[sym_id];

	const uint16_t offset = 8 * font_width_bytes- width;
	uint32_t line = 0;

	for(uint16_t i = 0; i < height; ++i) {
		const uint8_t* pchar = (uint8_t*)sym_data + font_width_bytes * i;
		switch(font_width_bytes) {
			case 1:
				line = pchar[0];
				break;

			case 2:
				line = (pchar[0] << 8) | pchar[1];
				break;

			case 3:
			default:
				line = (pchar[0]<< 16) | (pchar[1]<< 8) | pchar[2];
		}

		for (uint16_t j = 0; j < width; ++j) {
			if(line & (1 << (width - j + offset - 1))) {
				drawPixel(x+j, y+i, properties_.text_color);
			} else {
				drawPixel(x+j, y+i, properties_.background_color);
			}
		}
	}

}

void LCDDisplay::drawString(uint16_t x, uint16_t y, const char *text) {
	const char* curr_sym = text;
	uint16_t curr_x = x;
	while(*curr_sym != 0) {
		drawChar(curr_x, y, *curr_sym);

		curr_x += properties_.font->width;
		++curr_sym;
	}
}


void LCDDisplay::setCursor(uint16_t x, uint16_t y) {
	uint8_t parameter[4];

	//CASET Column Address Set
	parameter[0] = 0x00;
	parameter[1] = 0x00 + x;
	parameter[2] = 0x00;
	parameter[3] = 0xEF + x;
	writeRegData(ST7789H2::CASET, parameter, 4);

	parameter[0] = 0x00;
	parameter[1] = 0x00 + y;
	parameter[2] = 0x00;
	parameter[3] = 0xEF + y;
	writeRegData(ST7789H2::RASET, parameter, 4);
}

void LCDDisplay::drawPixel(uint16_t x, uint16_t y, Color color) {
	setCursor(x, y);
	writeReg(ST7789H2::WRITE_RAM);
	writeData(color);
}


void LCDDisplay::setBackgroundColor(Color color) {
	properties_.background_color = color;
}

void LCDDisplay::setTextColor(Color color) {
	properties_.text_color = color;
}

uint8_t LCDDisplay::backlight() {
	return level_;
}

uint16_t LCDDisplay::id() {
	return readReg(ST7789H2::LCD_ID);
}

uint16_t LCDDisplay::readData() {
	return io_settings_.lcd_bank->ram;
}

uint16_t LCDDisplay::readReg(uint8_t reg) {
	writeReg(reg);
	readData();
	return static_cast<uint8_t>(readData());
}

void LCDDisplay::clear(Color color) {
	for(uint16_t i = 0; i < ST7789H2::LCD_HEIGHT; ++i) {
		drawHLine(0, i, ST7789H2::LCD_WIDTH, color);
	}
}
