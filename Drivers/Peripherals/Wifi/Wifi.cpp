#include <Wifi/Wifi.hpp>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cstring>

wifi::Wifi::Wifi(WifiIOSettings io_settings)
:	spi_(io_settings)
{

}

wifi::Wifi::~Wifi()
{
}

std::vector<wifi::AP> wifi::Wifi::scan()
{
	std::vector<wifi::AP> res;

	char tmp[256] = {0};
	spi_.send("F0\r", 0xFFFF);
	spi_.receive(tmp, 2, 0xFFFF);

	int received = 0;
	bool completed = false;

	do
	{
		received += spi_.receive(tmp+received, 256-received, 0xFFFF);

		char* end = strstr(tmp, "\r\n");

		while(end)
		{
			uint8_t length = end-tmp;

			char* entry = new char[length+1];
			memcpy(entry, tmp, length);
			entry[length] = 0;
			if(strcmp(entry, "OK")==0)
			{
				delete[] entry;
				completed = true;
				break;
			}

			received -= length+2;

			memmove(tmp, end+2, received);
			memset(tmp+received, 0, 256-received);
			// parse ap info

			wifi::AP ap_info;
			strtok(entry, ","); // Ignore

			char* ssid_unparsed = strtok(NULL, ",");
			// Ignore starting and ending "
			memset(ap_info.ssid, '\0', 33);
			strncpy(ap_info.ssid, ssid_unparsed + 1, strlen(ssid_unparsed) - 2);

			char* bssid_unparsed = strtok(NULL, ",");
			for(uint8_t i = 0; i < 6; ++i) {
				ap_info.bssid[i] = (uint8_t) strtol(bssid_unparsed, NULL, 16);
				bssid_unparsed += 3;
			}

			char* rssi_unparsed = strtok(NULL, ",");
			ap_info.rssi = (int8_t) atoi(rssi_unparsed);

			strtok(NULL, ","); // Ignore
			strtok(NULL, ","); // Ignore

			char* security_unparsed = strtok(NULL, ",");
			ap_info.security = wifi::parseSecurity(security_unparsed);

			strtok(NULL, ","); // Ignore

			char* channel_unparsed = strtok(NULL, ",");
			ap_info.channel = atoi(channel_unparsed);

			res.push_back(ap_info);

			delete[] entry;
			end = strstr(tmp, "\r\n");

		}
	}
	while(received > 0 && !completed);

	return res;
}

bool wifi::Wifi::connect(const char *ssid, const char *password, SECURITY security) {
	char buffer[256] = {0};

	//set ssid
	sprintf(buffer,"C1=%s\r", ssid);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}

	//set password
	sprintf(buffer, "C2=%s\r", password);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}

	//set security level
	sprintf(buffer, "C3=%u\r", (uint32_t)security);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}

	sprintf(buffer, "C0\r");
	spi_.send(buffer, 0xFFFF);

	uint8_t received = spi_.receive(buffer, 256, 0xFFFF);
	buffer[received] = 0;

	return strstr(buffer, "OK\r\n> ") != nullptr;
}

bool wifi::Wifi::set_dhcp_client(bool state) {
	char buffer[8];
	sprintf(buffer, "C4=%d\r", state ? 1 : 0);
	spi_.send(buffer, 0xFFFF);
	return check_response_ok();
}

bool wifi::Wifi::disconnect() {
	char* buffer = "CD\r";
	spi_.send(buffer, 0xFFFF);
	return check_response_ok();
}

bool wifi::Wifi::open(size_t id, SOCKET_TYPE type, const char *address, uint32_t port) {
#pragma warning "only tcp and udp support"
	static uint32_t random_local_port = 49512;
	char buffer[128] = {0};
	// set socket id
	sprintf(buffer, "P0=%d", id);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}

	//set protocol
	sprintf(buffer, "P1=%u", type);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}

	if(type == SOCKET_TYPE::TCP) {
		sprintf(buffer, "P2=%u", random_local_port);
		spi_.send(buffer, 0xFFFF);
		if(!check_response_ok()) {
			return false;
		}
	}

	sprintf(buffer, "P3=%s", address);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}

	if(type == SOCKET_TYPE::UDP) {
		sprintf(buffer, "P4=%u", random_local_port);
		spi_.send(buffer, 0xFFFF);
		if(!check_response_ok()) {
			return false;
		}
	} else {
		sprintf(buffer, "P4=%u", port);
		spi_.send(buffer, 0xFFFF);
		if(!check_response_ok()) {
			return false;
		}
	}

	sprintf(buffer, "P5=0");
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}

	// start connection
	sprintf(buffer, "P6=1");
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}

	if(type == SOCKET_TYPE::UDP) {
		sprintf(buffer, "P0=%u", id);
		spi_.send(buffer, 0xFFFF);
		if(!check_response_ok()) {
			return false;
		}

		sprintf(buffer, "P4=%u", port);
		spi_.send(buffer, 0xFFFF);
		if(!check_response_ok()) {
			return false;
		}
	}
	return true;
}

bool wifi::Wifi::close(size_t id) {
	char buffer[8] = {0};

	sprintf(buffer, "P0=%u", id);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}

	// start connection
	sprintf(buffer, "P6=0");
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}

	return true;
}

bool wifi::Wifi::check_response_ok() {
	char buffer[128] = {0};
	uint8_t received = spi_.receive(buffer, 128, 0xFFFF);
	buffer[received] = 0;
	char* ok_response = strstr(buffer, "OK\r\n");
	if(ok_response == nullptr) {
		return false;
	}
	char *prompt = strstr(ok_response+4, "> ");
	if(prompt == nullptr) {
		return false;
	}

	return true;
}
