#ifndef TOUCH_CONSTANTS_HPP_
#define TOUCH_CONSTANTS_HPP_

#include "stdint.h"


namespace touch {
	constexpr uint8_t MAX_DETECTABLE_TOUCH = 2;
	constexpr uint8_t DEFAULT_THRESHHOLD = 20;

	enum class ID {
		UNKNOWN	 	= 0x00,
		FT6206		= 0x11,
		FT6x36		= 0xCD
	};

	namespace detail {
		enum class REG: uint16_t {
			MODE 		  		= 0x00,
			ID 					= 0xA8,
			GMODE 				= 0xA4,
			TD_STAT_REG 		= 0x02,
			THRESHHOLD 			= 0x80,
			P1_XH  				= 0x03,
			P2_XH				= 0x09,
			P1_WEIGHT			= 0x07,
			P2_WEIGHT			= 0x0d,
			GEST_ID				= 0x01,
			RADIAN_VALUE		= 0x91,
			OFFSET_LEFT_RIGHT	= 0x92,
			OFFSET_UP_DOWN		= 0x93,
			DISTANCE_LEFT_RIGHT = 0x94,
			DISTANCE_UP_DOWN	= 0x95,
			DISTANCE_ZOOM		= 0x96,
		};

		namespace FLAG {
			constexpr uint8_t G_MODE_INTERRUPT_TRIGGER = 0x01;
			constexpr uint8_t G_MODE_INTERRUPT_POLLING = 0x00;
			constexpr uint8_t G_MODE_INTERRUPT_MASK    = 0x03;
			constexpr uint8_t TD_STAT_MASK             = 0x0F;
			constexpr uint8_t MSB_MASK         		   = 0x0F;
			constexpr uint8_t LSB_MASK 				   = 0xFF;
			constexpr uint8_t TOUCH_AREA_MASK		   = (0x04 << 4);
			constexpr uint8_t TOUCH_AREA_SHIFT		   = 0x04;
			constexpr uint8_t EVENT_FLAG_MASK		   = (3 << 6);
			constexpr uint8_t EVENT_FLAG_SHIFT		   = 0x06;
			constexpr uint8_t PSENSOR_ENABLE_MASK  = 0x01;
		};
	}

}


#endif /* TOUCH_CONSTANTS_HPP_ */
