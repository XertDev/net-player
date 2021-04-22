#include <Wifi/Wifi.hpp>
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
	char tmp[256] = {0};
	spi_.send("F0\r", 0xFFFF);
	spi_.receive(tmp, 2, 0xFFFF);

	int received = 0;
	bool completed = false;

	bool first = true;
	do
	{
		if(first)
		{
			received = spi_.receive(tmp+received, 256-received, 0xFFFF);
			first = false;
		}
		else
		{
			received = spi_.receive(tmp+received-3, 256-received, 0xFFFF);
		}

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

			received -= length+1;

			memmove(tmp, end+2, received);
			memset(tmp+received, 0, 256-received);
			//parse ap info
			delete[] entry;
			end = strstr(tmp, "\r\n");

		}
	}
	while(received > 0 && !completed);

}
