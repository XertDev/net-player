#include <Wifi/Wifi.hpp>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <algorithm>

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

	std::sort(res.begin(), res.end(), [](const auto& wifi1, const auto& wifi2) {
		return wifi1.rssi > wifi2.rssi;
	});

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

wifi::Socket* wifi::Wifi::open(size_t id, SOCKET_TYPE type, const char *address, uint32_t port) {
#pragma warning "only tcp and udp support"
	static uint32_t random_local_port = 49512;
	char buffer[128] = {0};
	// set socket id
	sprintf(buffer, "P0=%d\r", id);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return nullptr;
	}

	//set protocol
	sprintf(buffer, "P1=%u\r", type);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return nullptr;
	}

	if(type == SOCKET_TYPE::TCP) {
		sprintf(buffer, "P2=%u\r", random_local_port);
		spi_.send(buffer, 0xFFFF);
		if(!check_response_ok()) {
			return nullptr;
		}
	}

	sprintf(buffer, "P3=%s\r", address);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return nullptr;
	}

	if(type == SOCKET_TYPE::UDP) {
		sprintf(buffer, "P4=%u\r", random_local_port);
		spi_.send(buffer, 0xFFFF);
		if(!check_response_ok()) {
			return nullptr;
		}
	} else {
		sprintf(buffer, "P4=%u\r", port);
		spi_.send(buffer, 0xFFFF);
		if(!check_response_ok()) {
			return nullptr;
		}
	}

	sprintf(buffer, "P5=0\r");
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return nullptr;
	}

	// start connection
	sprintf(buffer, "P6=1\r");
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return nullptr;
	}

	if(type == SOCKET_TYPE::UDP) {
		sprintf(buffer, "P0=%u\r", id);
		spi_.send(buffer, 0xFFFF);
		if(!check_response_ok()) {
			return nullptr;
		}

		sprintf(buffer, "P4=%u\r", port);
		spi_.send(buffer, 0xFFFF);
		if(!check_response_ok()) {
			return nullptr;
		}
	}
	return new Socket(this, id);
}


size_t wifi::Wifi::ping(const char *address, size_t count) {
	char buffer[128] = {0};
	sprintf(buffer, "T1=%s\r", address);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}
	sprintf(buffer, "T2=%u\r", count-1);
	spi_.send(buffer, 0xFFFF);
	if(!check_response_ok()) {
		return false;
	}
	sprintf(buffer, "T0\r");
	spi_.send(buffer, 0xFFFF);
	spi_.receive(buffer, 2, 0xFFFF);

	int received = 0;
	bool completed = false;
	uint8_t succesfull = 0;

	do
	{
		received += spi_.receive(buffer+received, 128-received, 0xFFFF);

		char* end = strstr(buffer, "\r\n");

		while(end)
		{
			uint8_t length = end-buffer;

			char* entry = new char[length+1];
			memcpy(entry, buffer, length);
			entry[length] = 0;
			if(strcmp(entry, "OK")==0)
			{
				delete[] entry;
				completed = true;
				break;
			}

			strtok(entry, ",");
			char* time = strtok(nullptr, ",");
			if(!strstr(time, "Timeout"))
			{
				++succesfull;
			}

			received -= length+2;

			memmove(buffer, end+2, received);
			memset(buffer+received, 0, 128-received);


			delete[] entry;
			end = strstr(buffer, "\r\n");

		}
	}
	while(received > 0 && !completed);

	return succesfull;
}

const char* wifi::Wifi::get_ip(const char* domain_name) {
	char buffer[256]={};
	sprintf(buffer, "D0=%s\r", domain_name);
	spi_.send(buffer, 0xFFFF);
	spi_.receive(buffer, 256, 0xFFFF);

	char* start = strstr(buffer, "\r\n") + 2;
	char* end = strstr(start, "\r\n");
	char* res = (char*) malloc(end - start + 1);
	*end = 0;
	sprintf(res, "%s", start);

	return res;
}

uint32_t wifi::Wifi::getUTCTime() {
	char buffer[32] = {0};
	sprintf(buffer, "GT\r");
	spi_.send(buffer, 0xFFFF);
	spi_.receive(buffer, 2, 0xFFFF);

	size_t received = spi_.receive(buffer, 0, 0xFFFF);
	buffer[received] = 0;
	char* end = strstr(buffer, "\r\n");
	*end = 0;

	return atol(buffer);
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
