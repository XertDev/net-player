#ifndef TOUCH_TOUCHPANEL_HPP_
#define TOUCH_TOUCHPANEL_HPP_

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include "Constants.hpp"
#include "TouchPoint.hpp"
#include "TouchDetails.hpp"

namespace touch {

	enum class TouchPanelError {
		OK = 0,
		UNKNOWN_ERROR
	};

	struct TouchState {
		bool detected;
		uint16_t x[MAX_DETECTABLE_TOUCH];
		uint16_t y[MAX_DETECTABLE_TOUCH];

		uint8_t weight[MAX_DETECTABLE_TOUCH];
		uint8_t event_id[MAX_DETECTABLE_TOUCH];

		uint8_t area[MAX_DETECTABLE_TOUCH];
		uint32_t gesture_id;
	};

	class TouchPanel {
	public:
		TouchPanel(FMPI2C_HandleTypeDef *fmpi2c_handler, uint8_t address, uint8_t h, uint8_t w, void(*reset_func)());
		~TouchPanel();

		uint16_t id();

		void setTriggerMode();
		void setPollingMode();

		void setThreshhold(uint8_t threshhold);

		uint8_t detectTouch();

		TouchPoint getPoint(uint8_t index);
		TouchDetails getDetails(uint8_t index);
		uint8_t getGesture();
	private:

		FMPI2C_HandleTypeDef *fmpi2c_handler_;
		uint8_t address_;
		uint8_t h_;
		uint8_t w_;

		void(*reset_func_)();



		void writeReg(detail::REG reg, uint8_t* buff, uint16_t count);
		uint8_t readReg(detail::REG reg);
		uint16_t readRegMultiple(detail::REG reg, uint8_t *buff, uint16_t count);

		void resetFMPI2C();
	};
}

#endif /* TOUCH_TOUCHPANEL_HPP_ */
