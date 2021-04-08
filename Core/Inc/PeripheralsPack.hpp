#ifndef INC_PERIPHERALSPACK_HPP_
#define INC_PERIPHERALSPACK_HPP_

#include "LCD/LCDDisplay.hpp"
#include "Storage/Storage.hpp"
#include "AudioCodec/AudioCodec.hpp"



struct PeripheralsPack
{
	LCDDisplay lcd_display;
	Storage storage;
	audio::AudioCodec codec;
};


#endif /* INC_PERIPHERALSPACK_HPP_ */
