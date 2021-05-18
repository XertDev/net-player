#include <Wifi/Socket.hpp>
#include "Wifi.hpp"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>


wifi::Socket::Socket(wifi::Wifi *wifi, uint8_t socket_id)
: wifi_(wifi), socket_id_(socket_id) {
}

char* wifi::Socket::read(size_t size, size_t& length) {
	assert(size > 0 && size <= 1460);
		char temp[128] = {0};
		// set socket id
		sprintf(temp, "P0=%d\r", socket_id_);
		wifi_->spi_.send(temp, 0xFFFF);
		if(!wifi_->check_response_ok()) {
			return nullptr;
		}

		char buffer[256];
		sprintf(buffer, "R1=%u\r", size);
		wifi_->spi_.send(buffer, 0xFFFF);
		if(!wifi_->check_response_ok()) {
			return nullptr;
		}

		sprintf(buffer, "R0\r");
		wifi_->spi_.send(buffer, 0xFFFF);
		wifi_->spi_.receive(buffer, 2, 0xFFFF);

		char* response = (char*) malloc(size + 8);
		uint32_t received = wifi_->spi_.receive(response, size+8, 0xFFFF);

		size_t real_length = 0;
		for(; real_length < received - 8; ++ real_length) {
			char* search_start = response + real_length;
			if(strncmp(search_start, "\r\nOK\r\n", 8) == 0) {
				break;
			}
		}

		length = real_length;
		if(real_length == 0) {
			return nullptr;
		} else {
			char* real_response = (char*)malloc(real_length);
			memcpy(real_response, response, real_length);
			free(response);
			return real_response;
		}
}

bool wifi::Socket::send(char* buffer, size_t size) {
	char temp[128] = {0};
	// set socket id
	sprintf(temp, "P0=%d\r", socket_id_);
	wifi_->spi_.send(temp, 0xFFFF);
	if(!wifi_->check_response_ok()) {
		return false;
	}

	int start_size = sprintf(temp, "S3=%u\r", size);
	if(start_size%2 == 1) {
		temp[start_size] = buffer[0];
		temp[start_size+1] = 0;
		wifi_->spi_.send(temp, 0xFFFF);
		wifi_->spi_.send(buffer+1, size-1, 0xFFFF);
	} else {
		wifi_->spi_.send(temp, 0xFFFF);
		wifi_->spi_.send(buffer, size, 0xFFFF);
	}
	return wifi_->check_response_ok();
}

wifi::Socket::~Socket() {
	// TODO Auto-generated destructor stub
}

bool wifi::Socket::close() {
	char buffer[8] = {0};

	sprintf(buffer, "P0=%u\r", socket_id_);
	wifi_->spi_.send(buffer, 0xFFFF);
	if(!wifi_->check_response_ok()) {
		return false;
	}

	// start connection
	sprintf(buffer, "P6=0\r");
	wifi_->spi_.send(buffer, 0xFFFF);
	if(!wifi_->check_response_ok()) {
		return false;
	}

	return true;
}
