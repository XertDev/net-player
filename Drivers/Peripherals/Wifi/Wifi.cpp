#include <Wifi/Wifi.hpp>
#include <cstdlib>
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
