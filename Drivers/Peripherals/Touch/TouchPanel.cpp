#include "TouchPanel.hpp"
#include "stm32f4xx_hal_fmpi2c.h"

constexpr auto I2C_MEMADD_SIZE_8BIT = 0x00000001U;

touch::TouchPanel::TouchPanel(FMPI2C_HandleTypeDef *fmpi2c_handler, uint8_t address, uint8_t h, uint8_t w, void(*reset_func)())
:fmpi2c_handler_(fmpi2c_handler), address_(address),h_(h), w_(w), reset_func_(reset_func) {

}

touch::TouchPanel::~TouchPanel() {

}

void touch::TouchPanel::writeReg(detail::REG reg, uint8_t *buff, uint16_t count) {
	auto status = HAL_OK;

	status = HAL_FMPI2C_Mem_Write(fmpi2c_handler_, address_, (uint16_t)reg, I2C_MEMADD_SIZE_8BIT, buff, count, 1000);

	if(status != HAL_OK) {
		resetFMPI2C();
	}
}

uint16_t touch::TouchPanel::id() {
	return readReg(detail::REG::ID);
}

bool touch::TouchPanel::calibrate() {
	using namespace detail::FLAG;

	uint8_t value[1] = {0};

	value[0] = MODE_FACTORY << MODE_SHIFT;
	writeReg(detail::REG::MODE, value, 1);

	uint8_t state = readReg(detail::REG::MODE);
	state = (state & (MODE_MASK << MODE_SHIFT)) >> MODE_SHIFT;
	if(state != MODE_FACTORY) {
		return false;
	}

	value[0] = 0x04;
	writeReg(detail::REG::TD_STAT_REG, value, 1);

	HAL_Delay(300);

	bool ended = false;

	for(uint8_t attempts = 0; attempts < 100; ++attempts) {
		state = readReg(detail::REG::MODE);
		state = (state & (MODE_MASK << MODE_SHIFT)) >> MODE_SHIFT;

		if(state == MODE_WORKING) {
			ended = true;
			break;
		}

		HAL_Delay(200);
	}

	if(!ended) {
		return false;
	}

	//go to factory mode
	value[0] = (MODE_FACTORY & MODE_MASK) << MODE_SHIFT;
	writeReg(detail::REG::MODE, value, 1);
	HAL_Delay(200);

	//store calibration result
	value[0] = 0x05;
	writeReg(detail::REG::TD_STAT_REG, value, 1);
	HAL_Delay(300);

	//go to work mode
	value[0] = (MODE_WORKING & MODE_MASK) << MODE_SHIFT;
	writeReg(detail::REG::MODE, value, 1);
	HAL_Delay(300);

	return true;
}

uint8_t touch::TouchPanel::readReg(detail::REG reg) {
	uint8_t value[1] = {0};

	readRegMultiple(reg, value, 1);

	return value[0];
}

uint16_t touch::TouchPanel::readRegMultiple(detail::REG reg, uint8_t *buff, uint16_t count) {
	auto status = HAL_OK;
	status = HAL_FMPI2C_Mem_Read(fmpi2c_handler_, address_, (uint16_t)reg, I2C_MEMADD_SIZE_8BIT, buff, count, 1000);

	if(status) {
		resetFMPI2C();
	}

	return status;
}

void touch::TouchPanel::setTriggerMode() {
	using namespace detail::FLAG;
	uint8_t value = G_MODE_INTERRUPT_TRIGGER;

	writeReg(detail::REG::GMODE, &value, 1);
}

void touch::TouchPanel::setPollingMode() {
	using namespace detail::FLAG;
	uint8_t value = G_MODE_INTERRUPT_POLLING;

	writeReg(detail::REG::GMODE, &value, 1);
}

uint8_t touch::TouchPanel::detectTouch() {
	uint8_t touch_count = 0;

	touch_count = readReg(detail::REG::TD_STAT_REG);
	touch_count &= detail::FLAG::TD_STAT_MASK;

	if(touch_count > MAX_DETECTABLE_TOUCH) {
		touch_count = 0;
	}

	return touch_count;
}

void touch::TouchPanel::setThreshhold(uint8_t threshhold) {
	writeReg(detail::REG::THRESHHOLD, &threshhold, 1);
}

touch::TouchPoint touch::TouchPanel::getPoint(uint8_t index) {
	assert_param(index <= 1);

	using detail::REG;
	using namespace detail::FLAG;

	uint8_t raw_position[4];

	switch (index) {
	case 0:
		readRegMultiple(REG::P1_XH, raw_position, sizeof(raw_position));
		break;
	case 1:
		readRegMultiple(REG::P2_XH, raw_position, sizeof(raw_position));
		break;
	}

	TouchPoint point;
	point.y = (((raw_position[0] & MSB_MASK) << 8) | (raw_position[1] & LSB_MASK));
	point.x = 240 - (((raw_position[2] & MSB_MASK) << 8) | (raw_position[3] & LSB_MASK));

	return point;
}

touch::TouchDetails touch::TouchPanel::getDetails(uint8_t index) {
	assert_param(index <=1);

	using detail::REG;
	using namespace detail::FLAG;

	uint8_t raw_data[2];
	uint8_t event_id_raw;

	switch (index) {
	case 0:
		readRegMultiple(REG::P1_WEIGHT, raw_data, sizeof(raw_data));
		event_id_raw = readReg(REG::P1_XH);
		break;
	case 1:
		readRegMultiple(REG::P2_WEIGHT, raw_data, sizeof(raw_data));
		event_id_raw = readReg(REG::P2_XH);
		break;
	}

	TouchDetails details;
	details.weight = raw_data[0];
	details.area = (raw_data[1] & TOUCH_AREA_MASK) >> TOUCH_AREA_SHIFT;
	details.event_type = (event_id_raw & EVENT_FLAG_MASK) >> EVENT_FLAG_SHIFT;

	return details;
}

uint8_t touch::TouchPanel::getGesture() {
	return readReg(detail::REG::GEST_ID);
}

void touch::TouchPanel::resetFMPI2C() {
	reset_func_();
}
