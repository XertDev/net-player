#include <Wifi/Socket.hpp>
#include "Wifi.hpp"

#include <cstdio>


wifi::Socket::Socket(wifi::Wifi *wifi, uint8_t socket_id)
: wifi_(wifi), socket_id_(socket_id) {
}

char* wifi::Socket::read(size_t size) {
	char temp[128] = {0};
	// set socket id
	sprintf(temp, "P0=%d\r", socket_id_);
	wifi_->spi_.send(temp, 0xFFFF);
	if(!wifi_->check_response_ok()) {
		return nullptr;
	}

	//todo: implement this

	return nullptr;
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
