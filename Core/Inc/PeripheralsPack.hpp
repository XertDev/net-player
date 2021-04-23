#ifndef INC_PERIPHERALSPACK_HPP_
#define INC_PERIPHERALSPACK_HPP_

#include "LCD/LCDDisplay.hpp"
#include "Storage/Storage.hpp"
#include "AudioCodec/AudioCodec.hpp"
#include "Touch/TouchPanel.hpp"
#include "Wifi/Wifi.hpp"



struct PeripheralsPack
{
	LCDDisplay lcd_display;
	touch::TouchPanel touch_panel;
	Storage storage;
	audio::AudioCodec codec;
	wifi::Wifi wifi;
};


#endif /* INC_PERIPHERALSPACK_HPP_ */
