#include "Constants.hpp"
#include "stm32f4xx_hal.h"

audio::FREQUENCY audio::parseFreq(uint32_t freq) {
	// probably should return sth else on default
	if(freq > 96000) {
		return FREQUENCY::FREQ_96K;
	} else if(freq > 48000) {
		return FREQUENCY::FREQ_48K;
	} else if(freq > 44000) {
		return FREQUENCY::FREQ_44K;
	} else if(freq > 32000) {
		return FREQUENCY::FREQ_32K;
	} else if(freq > 22000) {
		return FREQUENCY::FREQ_22K;
	} else if(freq > 16000) {
		return FREQUENCY::FREQ_16K;
	} else if(freq > 11000) {
		return FREQUENCY::FREQ_11K;
	} else {
		return FREQUENCY::FREQ_8K;
	}
}

uint32_t audio::parseFreqSys(uint32_t freq) {
	if(freq > 96000) {
		return I2S_AUDIOFREQ_96K;
	} else if(freq > 48000) {
		return I2S_AUDIOFREQ_48K;
	} else if(freq > 44000) {
		return I2S_AUDIOFREQ_44K;
	} else if(freq > 32000) {
		return I2S_AUDIOFREQ_32K;
	} else if(freq > 22000) {
		return I2S_AUDIOFREQ_22K;
	} else if(freq > 16000) {
		return I2S_AUDIOFREQ_16K;
	} else if(freq > 11000) {
		return I2S_AUDIOFREQ_11K;
	} else if(freq > 8000){
		return I2S_AUDIOFREQ_8K;
	} else {
		return I2S_AUDIOFREQ_DEFAULT;
	}
}
