#ifndef LCD_LCDCONSTANTS_HPP_
#define LCD_LCDCONSTANTS_HPP_

namespace ST7789H2 {

enum class ORIENTATION: uint8_t {
	PORTRAIT = 0x00,
	LANDSCAPE = 0x01,
	LANDSCAPE_ROT180 = 0x02
};

constexpr uint16_t LCD_WIDTH        = 240;
constexpr uint16_t LCD_HEIGHT       = 240;

constexpr uint8_t ID                = 0x85;

constexpr uint8_t LCD_ID            = 0x04;
constexpr uint8_t SLEEP_IN          = 0x10;
constexpr uint8_t SLEEP_OUT         = 0x11;
constexpr uint8_t PARTIAL_DISPLAY   = 0x12;
constexpr uint8_t DISPLAY_INVERSION = 0x21;
constexpr uint8_t DISPLAY_ON        = 0x29;
constexpr uint8_t WRITE_RAM         = 0x2C;
constexpr uint8_t READ_RAM          = 0x2E;
constexpr uint8_t CASET             = 0x2A;
constexpr uint8_t RASET             = 0x2B;
constexpr uint8_t VSCRDEF           = 0x33; /* Vertical Scroll Definition */
constexpr uint8_t VSCSAD            = 0x37; /* Vertical Scroll Start Address of RAM */
constexpr uint8_t TEARING_EFFECT    = 0x35;
constexpr uint8_t NORMAL_DISPLAY    = 0x36;
constexpr uint8_t IDLE_MODE_OFF     = 0x38;
constexpr uint8_t IDLE_MODE_ON      = 0x39;
constexpr uint8_t COLOR_MODE        = 0x3A;
constexpr uint8_t PORCH_CTRL        = 0xB2;
constexpr uint8_t GATE_CTRL         = 0xB7;
constexpr uint8_t VCOM_SET          = 0xBB;
constexpr uint8_t DISPLAY_OFF       = 0xBD;
constexpr uint8_t LCM_CTRL          = 0xC0;
constexpr uint8_t VDV_VRH_EN        = 0xC2;
constexpr uint8_t VDV_SET           = 0xC4;
constexpr uint8_t VCOMH_OFFSET_SET  = 0xC5;
constexpr uint8_t FR_CTRL           = 0xC6;
constexpr uint8_t POWER_CTRL        = 0xD0;
constexpr uint8_t PV_GAMMA_CTRL     = 0xE0;
constexpr uint8_t NV_GAMMA_CTRL     = 0xE1;

}

#endif /* LCD_LCDCONSTANTS_HPP_ */
