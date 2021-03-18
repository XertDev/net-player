
#ifndef AUDIOCODEC_CONSTANTS_HPP_
#define AUDIOCODEC_CONSTANTS_HPP_

#include <stdint.h>

namespace audio {
	enum class OUTPUT_DEVICE: uint8_t {
		SPEAKER = 0,
		HEADPHONE,
		BOTH,
	};

	enum class FREQUENCY: uint8_t {
		FREQ_8K = 0,
		FREQ_16K,
		FREQ_32K,
		FREQ_48K,
		FREQ_96K,
		FREQ_11K,
		FREQ_22K,
		FREQ_44K
	};

	namespace detail {
		enum class REG: uint16_t {
			ID							= 0x00,
			WORKAROUND_1				= 0x102,
			WORKAROUND_2				= 0x817,
			ANTI_POP 					= 0x39,
			POWER_MANAGEMENT_1			= 0x01,
			POWER_MANAGEMENT_3			= 0x03,
			POWER_MANAGEMENT_5			= 0x05,
			DAC1_LEFT_MIXER_ROUTING		= 0x601,
			DAC1_RIGHT_MIXER_ROUTING	= 0x602,
			DAC2_LEFT_MIXER_ROUTING		= 0x604,
			DAC2_RIGHT_MIXER_ROUTING	= 0x605,
			AIF1_RATE					= 0x210,
			AIF1_MASTER_SLAVE			= 0x302,
			CLOCKING_1					= 0x208,
			AIF1_CLOCKING_1				= 0x200,
			SPKMIXL_ATTENUATION			= 0x22,
			SPKMIXR_ATTENUATION			= 0x23,
			SPKEAKER_MIXERS				= 0x36,
			CLASS_W						= 0x51,
			ANALOGUE_HP 				= 0x60,
			CHARGE_PUMP					= 0x4c,
			OUTPUT_MIXER_1				= 0x2d,
			OUTPUT_MIXER_2				= 0x2e,
			DC_SERVO					= 0x54,
			DAC1_LEFT_VOLUME			= 0x610,
			DAC1_RIGHT_VOLUME			= 0x611,
			AIF1_DAC1_FILTERS_1			= 0x420,
			DAC2_LEFT_VOLUME			= 0x612,
			DAC2_RIGHT_VOLUME			= 0x613,
			AIF1_DAC2_FILTERS_1			= 0x422,
			LEFT_OUTPUT_VOLUME			= 0x1C,
			RIGHT_OUTPUT_VOLUME			= 0x1D,
			SPEAKER_VOLUME_LEFT			= 0x26,
			SPEAKER_VOLUME_RIGHT		= 0x27,
			AIF1_CONTROL_1				= 0x300,
		};
	}
}


#endif /* AUDIOCODEC_CONSTANTS_HPP_ */
