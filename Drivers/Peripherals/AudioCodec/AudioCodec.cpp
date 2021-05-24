#include "AudioCodec.hpp"
#include "stm32f4xx_hal_fmpi2c.h"


audio::AudioCodec::AudioCodec(FMPI2C_HandleTypeDef* hfmpi2c1, I2S_HandleTypeDef* hi2s2, uint8_t address, void(*reset_func)())
:hfmpi2c1_(hfmpi2c1), hi2s2_(hi2s2), address_(address), reset_func_(reset_func) {
	// TODO Auto-generated constructor stub

}

audio::AudioCodec::~AudioCodec() {
	// TODO Auto-generated destructor stub
}

void audio::AudioCodec::init(OUTPUT_DEVICE output, FREQUENCY freq) {
	using detail::REG;

	uint16_t buff[16] = {};
	//generate some clock on codec to initialize registers
	HAL_I2S_Transmit_DMA(hi2s2_, buff, sizeof(buff));

	//reset
	writeReg(REG::ID, 0x8994);

	//workaround from errata
	writeReg(REG::WORKAROUND_1, 0x0003);
	writeReg(REG::WORKAROUND_2, 0x0000);
	writeReg(REG::WORKAROUND_1, 0x0000);

	//enable VMID soft start Start-up bias current enabled
	writeReg(REG::ANTI_POP, 0x006C);

	//enable bias generator
	writeReg(REG::POWER_MANAGEMENT_1, 0x0003);

	HAL_Delay(50);

	setOutputDevice(output);
	setFrequency(freq);

	//AIF1 word length = 16 bits, format I2S
	writeReg(REG::AIF1_CONTROL_1, 0x4010);

	//slave mode
	writeReg(REG::AIF1_MASTER_SLAVE, 0x0000);
	//enable DSP clock for AIF1 and core clock
	writeReg(REG::CLOCKING_1, 0x000A);
	//enable AIF1 clock, AIF1 clock source = MCLK1
	writeReg(REG::AIF1_CLOCKING_1, 0x0001);

	initOutput();

	HAL_I2S_DMAStop(hi2s2_);
}

void audio::AudioCodec::initOutput() {
	using detail::REG;

	//enable SPKRVOL PGA, SPKMIXR, SPKLVOL PGA, SPKMIXL
	writeReg(REG::POWER_MANAGEMENT_3, 0x0300);
	//left speaker mixer volume 0db
	writeReg(REG::SPKMIXL_ATTENUATION, 0x0000);
	//right speaker mixer volume 0db, output mode class D
	writeReg(REG::SPKMIXR_ATTENUATION, 0x0000);
	//unmute DAC2(left) to SPKMIXL, unmute DAC2(right) to SPKMIXR
	writeReg(REG::SPKEAKER_MIXERS, 0x0300);
	//enable bias generator, VMID, SPKOUTL, SPKOUTR
	writeReg(REG::POWER_MANAGEMENT_1, 0x3003);

	//enable class W, class W envelope tracking = AIF1 timeslot 0
	writeReg(REG::CLASS_W, 0x0005);

	//enable bias generator, VMID, HPOUT1(left, right)
	writeReg(REG::POWER_MANAGEMENT_1, 0x0303| 0x3003);
	//enable HPOUT1 intermediate stages
	writeReg(REG::ANALOGUE_HP, 0x0022);
	//enable charge pump
	writeReg(REG::CHARGE_PUMP, 0x9f25);

	HAL_Delay(15);

	//DAC1(left) to HPOUT1LVOL
	writeReg(REG::OUTPUT_MIXER_1, 0x0001);
	//DAC1(right) to HPOUT1RVOL
	writeReg(REG::OUTPUT_MIXER_2, 0x0001);

	//enable MIXOUTL, MIXOUTR
	writeReg(REG::POWER_MANAGEMENT_3, 0x0030 | 0x0300);

	//enable DC servo trigger startup both channels
	writeReg(REG::DC_SERVO, 0x0033);

	HAL_Delay(250);

	//enable HPOUT1(left, right), remove clamps
	writeReg(REG::ANALOGUE_HP, 0x00EE);

	//unmutes

	//unmute DAC1 left
	writeReg(REG::DAC1_LEFT_VOLUME, 0x00C0);
	//unmute DAC1 right
	writeReg(REG::DAC1_RIGHT_VOLUME, 0x00C0);


	//unmute AIF1 timeslot 0 DAC
	writeReg(REG::AIF1_DAC1_FILTERS_1, 0x0000);

	//unmute DAC2 left
	writeReg(REG::DAC2_LEFT_VOLUME, 0x00C0);
	//unmute DAC2 right
	writeReg(REG::DAC1_RIGHT_VOLUME, 0x00C0);

	//unmute AIF1 timeslot 1 DAC
	writeReg(REG::AIF1_DAC2_FILTERS_1, 0x0000);

}

uint32_t audio::AudioCodec::id() {
	return readReg(detail::REG::ID);
}

void audio::AudioCodec::writeReg(detail::REG reg, uint16_t value) {
	uint16_t tmp = ((value >> 8) & 0x00ff) | ((value << 8) & 0xff00);
	auto status = HAL_OK;

	status = HAL_FMPI2C_Mem_Write(
			hfmpi2c1_, address_,
			(uint16_t)reg,
			FMPI2C_MEMADD_SIZE_16BIT,
			(uint8_t*)&tmp,
			2,
			1000
		);

	if(status != HAL_OK) {
		resetFMPI2C();
	}
}

uint16_t audio::AudioCodec::readReg(detail::REG reg) {
	uint16_t read_value = 0;
	auto status = HAL_OK;

	status = HAL_FMPI2C_Mem_Read(hfmpi2c1_, address_, (uint16_t) reg, FMPI2C_MEMADD_SIZE_16BIT, (uint8_t*)&read_value, 2, 1000);

	if(status != HAL_OK) {
		resetFMPI2C();
	}

	return ((read_value >> 8) & 0x00ff) | ((read_value << 8) & 0xff00);
}

void audio::AudioCodec::setOutputDevice(OUTPUT_DEVICE device) {
	using detail::REG;

	switch(device) {
	case OUTPUT_DEVICE::SPEAKER:
		//disable DAC1(left), DAC1(right); enable DAC2(left), DAC2(right)
		writeReg(REG::POWER_MANAGEMENT_5, 0x0C0C);

		//disable AIF1 Timeslot 0(left) to DAC1(left)
		writeReg(REG::DAC1_LEFT_MIXER_ROUTING, 0x0000);
		//disable AIF1 Timeslot 0(right) to DAC1(right)
		writeReg(REG::DAC1_RIGHT_MIXER_ROUTING, 0x0000);

		//enable Timeslot 1(left) to DAC2(left)
		writeReg(REG::DAC2_LEFT_MIXER_ROUTING, 0x0002);
		//enable Timeslot 1(right) to DAC2(right)
		writeReg(REG::DAC2_RIGHT_MIXER_ROUTING, 0x0002);
		break;

	case OUTPUT_DEVICE::HEADPHONE:
		//enable DAC1(left), DAC1(right); disable DAC2(left), DAC2(right)
		writeReg(REG::POWER_MANAGEMENT_5, 0x0303);

		//enable AIF1 Timeslot 0(left) to DAC1(left)
		writeReg(REG::DAC1_LEFT_MIXER_ROUTING, 0x0001);
		//enable AIF1 Timeslot 0(right) to DAC1(right)
		writeReg(REG::DAC1_RIGHT_MIXER_ROUTING, 0x0001);

		//disable Timeslot 1(left) to DAC2(left)
		writeReg(REG::DAC2_LEFT_MIXER_ROUTING, 0x0000);
		//disable Timeslot 1(right) to DAC2(right)
		writeReg(REG::DAC2_RIGHT_MIXER_ROUTING, 0x0000);
		break;

	case OUTPUT_DEVICE::BOTH:
		//enable DAC1(left), DAC1(right), DAC2(left), DAC2(right)
		writeReg(REG::POWER_MANAGEMENT_5, 0x0303 | 0x0C0C);

		//enable AIF1 Timeslot 0(left) to DAC1(left)
		writeReg(REG::DAC1_LEFT_MIXER_ROUTING, 0x0001);
		//enable AIF1 Timeslot 0(right) to DAC1(right)
		writeReg(REG::DAC1_RIGHT_MIXER_ROUTING, 0x0001);

		//enable Timeslot 1(left) to DAC2(left)
		writeReg(REG::DAC2_LEFT_MIXER_ROUTING, 0x0002);
		//enable Timeslot 1(right) to DAC2(right)
		writeReg(REG::DAC2_RIGHT_MIXER_ROUTING, 0x0002);
	}
}

void audio::AudioCodec::setFrequency(FREQUENCY freq) {
	using detail::REG;

	switch(freq) {
	case FREQUENCY::FREQ_8K:
		writeReg(REG::AIF1_RATE, 0x0003);
		break;
	case FREQUENCY::FREQ_16K:
		writeReg(REG::AIF1_RATE, 0x0033);
		break;
	case FREQUENCY::FREQ_32K:
		writeReg(REG::AIF1_RATE, 0x0063);
		break;
	case FREQUENCY::FREQ_48K:
		writeReg(REG::AIF1_RATE, 0x0083);
		break;
	case FREQUENCY::FREQ_96K:
		writeReg(REG::AIF1_RATE, 0x00A3);
		break;
	case FREQUENCY::FREQ_11K:
		writeReg(REG::AIF1_RATE, 0x0013);
		break;
	case FREQUENCY::FREQ_22K:
		writeReg(REG::AIF1_RATE, 0x0043);
		break;
	case FREQUENCY::FREQ_44K:
		writeReg(REG::AIF1_RATE, 0x0073);
		break;
	}
}

void audio::AudioCodec::setVolume(uint8_t volume) {
	if(volume <= 0) {
		mute();
		this->current_volume = 0;
	} else {
		using detail::REG;

		// Not sure which one is better (probably neither of theese)
		const uint8_t volume_val = (volume > 63) ? 63 : volume;
		// const uint8_t volume_val = ((volume > 100)? 100:(((volume * 63) / 100)));
		this->current_volume = volume_val;

		unmute();

		writeReg(REG::LEFT_OUTPUT_VOLUME, volume_val | 0x140);
		writeReg(REG::RIGHT_OUTPUT_VOLUME, volume_val | 0x140);

		writeReg(REG::SPEAKER_VOLUME_LEFT, volume_val | 0x140);
		writeReg(REG::SPEAKER_VOLUME_RIGHT, volume_val | 0x140);
	}

}
uint8_t audio::AudioCodec::getVolume() {
	return this->current_volume;
}

void audio::AudioCodec::mute() {
	using detail::REG;
	writeReg(REG::AIF1_DAC1_FILTERS_1, 0x0200);
	writeReg(REG::AIF1_DAC2_FILTERS_1, 0x0200);

}

void audio::AudioCodec::unmute() {
	using detail::REG;
	writeReg(REG::AIF1_DAC1_FILTERS_1, 0x0000);
	writeReg(REG::AIF1_DAC2_FILTERS_1, 0x0000);
}

void audio::AudioCodec::resetFMPI2C() {
	reset_func_();
}
