#ifndef PERIPHERALS_WIFI_WIFISPI_H_
#define PERIPHERALS_WIFI_WIFISPI_H_

#include "WifiIOSettings.hpp"

namespace wifi
{
	namespace detail
	{
		class WifiSPI
		{
		public:
			WifiSPI(WifiIOSettings io_settings);
			~WifiSPI();

			void init();

			size_t receive(char* data, uint16_t len, uint32_t timeout);
			size_t receive_fast(char* data, uint16_t len, uint32_t timeout);
			size_t send(const char* const data, uint16_t len, uint32_t timeout);
			size_t send(const char* const data, uint32_t timeout);

			bool execute(char* cmd, char* data, uint32_t timeout);
		private:
			WifiIOSettings io_settings_;


			void reset();
			void enable_nss();
			void disable_nss();
			bool is_cmddata_ready();

		};
	}
}

#endif /* PERIPHERALS_WIFI_WIFISPI_H_ */
