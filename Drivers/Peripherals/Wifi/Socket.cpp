#include <Wifi/Socket.hpp>
#include "Wifi.hpp"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>


wifi::Socket::Socket(wifi::Wifi *wifi, uint8_t socket_id)
: wifi_(wifi), socket_id_(socket_id) {
}

bool wifi::Socket::read(void* dest_buffer, size_t size, size_t& length) {
	assert(size > 0 && size <= 1460);
		char temp[128] = {0};
		// set socket id
		if(wifi_->current_socket != socket_id_) {
			sprintf(temp, "P0=%d\r", socket_id_);
			wifi_->spi_.send(temp, 0xFFFF);
			if(!wifi_->check_response_ok()) {
				return false;
			}
			wifi_->current_socket = socket_id_;
		}

		char buffer[256];
		if(wifi_->packet_size != size) {
			sprintf(buffer, "R1=%u\r", size);
			wifi_->spi_.send(buffer, 0xFFFF);
			if(!wifi_->check_response_ok()) {
				return false;
			}
			wifi_->packet_size = size;
		}

		if(wifi_->packet_timeout != 0) {
			sprintf(buffer, "R2=%u\r", 0);
			wifi_->spi_.send(buffer, 0xFFFF);
			if(!wifi_->check_response_ok()) {
				return false;
			}
			wifi_->packet_timeout = 0;
		}


		sprintf(buffer, "R0\r");
		wifi_->spi_.send(buffer, 0xFFFF);
		wifi_->spi_.receive(buffer, 2, 0xFFFF);

		uint32_t received = wifi_->spi_.receive((char*)dest_buffer, size, 0xFFFF);
		char* search_start = (char*)dest_buffer + received - 8;
		if(strncmp(search_start, "\r\nOK\r\n> ", 8) == 0) {
			length = size - 8;
			return true;
		}

		size_t real_length = received - 8;

		for(; 0 < real_length; --real_length) {
			char* search_start = (char*)dest_buffer + real_length;
			if(strncmp(search_start, "\r\nOK\r\n> ", 8) == 0) {
				length = real_length;
				return true;
			}
		}

		wifi_->spi_.receive(buffer, 8, 0xFFFF);

		if(strncmp(buffer, "\r\nOK\r\n> ", 8) == 0) {
			length = size;
			return true;
		}

//		char* response = (char*) malloc(size + 8);
//		uint32_t received = wifi_->spi_.receive(response, size+8, 0xFFFF);
//
//		size_t real_length = received - 8;
//		for(; 0 < real_length; --real_length) {
//			char* search_start = response + real_length;
//			if(strncmp(search_start, "\r\nOK\r\n> ", 8) == 0) {
//				break;
//			}
//		}
//
//		length = real_length;
//		if(real_length == 0) {
//			return false;
//		} else {
//			memcpy(dest_buffer, response, real_length);
//			free(response);
//			return true;
//		}
}

bool wifi::Socket::read_fast(void* dest_buffer, size_t size, size_t& length) {
	assert(size > 0 && size <= 1460);
		char temp[128] = {0};
		// set socket id
		if(wifi_->current_socket != socket_id_) {
			sprintf(temp, "P0=%d\r", socket_id_);
			wifi_->spi_.send(temp, 0xFFFF);
			if(!wifi_->check_response_ok()) {
				return false;
			}
			wifi_->current_socket = socket_id_;
		}

		char buffer[256];
		if(wifi_->packet_size != size) {
			sprintf(buffer, "R1=%u\r", size);
			wifi_->spi_.send(buffer, 0xFFFF);
			if(!wifi_->check_response_ok()) {
				return false;
			}
			wifi_->packet_size = size;
		}

		if(wifi_->packet_timeout != 0) {
			sprintf(buffer, "R2=%u\r", 0);
			wifi_->spi_.send(buffer, 0xFFFF);
			if(!wifi_->check_response_ok()) {
				return false;
			}
			wifi_->packet_timeout = 0;
		}

		sprintf(buffer, "R0\r");
		wifi_->spi_.send(buffer, 0xFFFF);
		wifi_->spi_.receive(buffer, 2, 0xFFFF);

		uint32_t received = wifi_->spi_.receive_fast((char*)dest_buffer, size, 0xFFFF);
		char* search_start = (char*)dest_buffer + received - 8;
		if(strncmp(search_start, "\r\nOK\r\n> ", 8) == 0) {
			length = size - 8;
			return true;
		}

		size_t real_length = received - 8;

		for(; 0 < real_length; --real_length) {
			char* search_start = (char*)dest_buffer + real_length;
			if(strncmp(search_start, "\r\nOK\r\n> ", 8) == 0) {
				length = real_length;
				return true;
			}
		}

		wifi_->spi_.receive_fast(buffer, 8, 0xFFFF);

		if(strncmp(buffer, "\r\nOK\r\n> ", 8) == 0) {
			length = size;
			return true;
		}
}

bool wifi::Socket::send(char* buffer, size_t size) {
	char temp[128] = {0};
	// set socket id
	if(wifi_->current_socket != socket_id_) {
		sprintf(temp, "P0=%d\r", socket_id_);
		wifi_->spi_.send(temp, 0xFFFF);
		if(!wifi_->check_response_ok()) {
			return false;
		}
		wifi_->current_socket = socket_id_;
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

	if(wifi_->current_socket != socket_id_) {
		sprintf(buffer, "P0=%d\r", socket_id_);
		wifi_->spi_.send(buffer, 0xFFFF);
		if(!wifi_->check_response_ok()) {
			return false;
		}
		wifi_->current_socket = socket_id_;
	}

	// start connection
	sprintf(buffer, "P6=0\r");
	wifi_->spi_.send(buffer, 0xFFFF);
	if(!wifi_->check_response_ok()) {
		return false;
	}

	return true;
}
