#include "../stm32f4xx_hal.h"
#include "main.h"
#include "PeripheralsPack.hpp"
#include "LCD/LCDDisplay.hpp"
#include "AudioCodec/AudioCodec.hpp"
#include "Touch/TouchPanel.hpp"
#include "StationInput/StationInput.hpp"
#include "RadioView/RadioView.hpp"


extern TIM_HandleTypeDef htim9;
extern SD_HandleTypeDef hsd;
extern FMPI2C_HandleTypeDef hfmpi2c1;
extern I2S_HandleTypeDef hi2s2;

extern bool detected_touch;

void resetFMPI2C() {
	HAL_FMPI2C_DeInit(&hfmpi2c1);
	  hfmpi2c1.Instance = FMPI2C1;
	  hfmpi2c1.Init.Timing = 0x00606092;
	  hfmpi2c1.Init.OwnAddress1 = 0;
	  hfmpi2c1.Init.AddressingMode = FMPI2C_ADDRESSINGMODE_7BIT;
	  hfmpi2c1.Init.DualAddressMode = FMPI2C_DUALADDRESS_DISABLE;
	  hfmpi2c1.Init.OwnAddress2 = 0;
	  hfmpi2c1.Init.OwnAddress2Masks = FMPI2C_OA2_NOMASK;
	  hfmpi2c1.Init.GeneralCallMode = FMPI2C_GENERALCALL_DISABLE;
	  hfmpi2c1.Init.NoStretchMode = FMPI2C_NOSTRETCH_DISABLE;
	  if (HAL_FMPI2C_Init(&hfmpi2c1) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  /** Configure Analogue filter
	  */
	  if (HAL_FMPI2CEx_ConfigAnalogFilter(&hfmpi2c1, FMPI2C_ANALOGFILTER_ENABLE) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

LCDIOSettings settings {
	htim9,
	TIM_CHANNEL_1,
	{LCD_TE_GPIO_Port, LCD_TE_Pin},
	{LCD_CTP_RST_GPIO_Port, LCD_CTP_RST_Pin},
	(LCDController*)(0x60000000 | 0x08000000)
};

uint8_t touches = 0;
uint8_t gesture = 0;
touch::TouchPoint point;
touch::TouchDetails details;

extern "C" void main_cpp();
void main_cpp() {
	/* Init peripherals */
	PeripheralsPack pack {
		LCDDisplay(settings),
		touch::TouchPanel(&hfmpi2c1, 0x70, 240, 240, &resetFMPI2C),
		Storage(),
		audio::AudioCodec(&hfmpi2c1, &hi2s2, 0x34, &resetFMPI2C)
	};

	pack.lcd_display.init();
	pack.lcd_display.setOrientation(ST7789H2::ORIENTATION::LANDSCAPE_ROT180);

	pack.touch_panel.setPollingMode();
	pack.touch_panel.id();
	pack.touch_panel.setThreshhold(20);

	pack.codec.init(audio::OUTPUT_DEVICE::HEADPHONE, audio::FREQUENCY::FREQ_44K);
	pack.codec.setVolume(100);

	uint8_t modes_stack[16] = {1, 0};
	void (*modes[])(uint8_t* modes_stack, PeripheralsPack& pack) =
	{
		radioView,
		stationInput
	};

	while(true) {
		uint8_t next;
		uint8_t* tmp = modes_stack;
		while(*tmp != 0) {
			next = *tmp;
			++tmp;
		}
		modes[next-1](modes_stack, pack);
	}
}
