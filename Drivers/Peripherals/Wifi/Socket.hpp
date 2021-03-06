#ifndef PERIPHERALS_WIFI_SOCKET_HPP_
#define PERIPHERALS_WIFI_SOCKET_HPP_

#include <vector>
#include <cstdint>


namespace wifi {
	class Wifi;


	class Socket {
	private:
		friend class Wifi;
		Socket(Wifi* wifi, uint8_t socket_id);

		Wifi* wifi_;

		uint8_t socket_id_;
	public:
		bool read(void* dest_buffer, size_t size, size_t& length);
		bool read_fast(void* dest_buffer, size_t size, size_t& length);
		bool send(char* buffer, size_t size);
		bool close();

		~Socket();


	};
}
#endif /* PERIPHERALS_WIFI_SOCKET_HPP_ */
