#ifndef AUDIOCODEC_AUDIOCODEC_HPP_
#define AUDIOCODEC_AUDIOCODEC_HPP_

#include "stm32f4xx_hal.h"
#include "Constants.hpp"

namespace audio {
	class AudioCodec {
	public:
		AudioCodec(FMPI2C_HandleTypeDef* hfmpi2c1, I2S_HandleTypeDef* hi2s2, uint8_t address, void(*reset_func)());
		~AudioCodec();

		uint32_t id();

		void init(OUTPUT_DEVICE output, FREQUENCY freq);

		void setVolume(uint8_t volume);
		void mute();
		void unmute();

	private:
		FMPI2C_HandleTypeDef* hfmpi2c1_;
		I2S_HandleTypeDef* hi2s2_;
		uint8_t address_;

		void(*reset_func_)();

		void writeReg(detail::REG reg, uint16_t value);
		uint16_t readReg(detail::REG reg);

		void setOutputDevice(OUTPUT_DEVICE device);
		void setFrequency(FREQUENCY freq);

		void initOutput();

		void resetFMPI2C();
	};
}

#endif /* AUDIOCODEC_AUDIOCODEC_HPP_ */
