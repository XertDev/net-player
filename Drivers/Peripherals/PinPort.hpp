
#ifndef PINPORT_HPP_
#define PINPORT_HPP_

#include "stm32f4xx_hal.h"

	struct PinPort {
		GPIO_TypeDef* GPIOx;
		uint16_t GPIO_Pin;
	};


#endif /* PINPORT_HPP_ */
