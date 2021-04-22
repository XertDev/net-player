#include <Wifi/WifiSPI.h>
#include <cassert>
#include <cstring>

wifi::detail::WifiSPI::WifiSPI(WifiIOSettings io_settings)
: io_settings_(io_settings)
{
	init();
}

wifi::detail::WifiSPI::~WifiSPI()
{
}

void wifi::detail::WifiSPI::init() {
	reset();
	enable_nss();

	uint8_t prompt[6];

	uint8_t index = 0;
	while(is_cmddata_ready())
	{
		assert(index < 6);
		HAL_SPI_Receive(&io_settings_.data, &prompt[index], 1, 0xFFFF);
		index+=2;
	}
	disable_nss();
}

void wifi::detail::WifiSPI::reset()
{
    HAL_GPIO_WritePin(io_settings_.reset.GPIOx, io_settings_.reset.GPIO_Pin, GPIO_PIN_RESET );
    HAL_Delay(10);
    HAL_GPIO_WritePin(io_settings_.reset.GPIOx, io_settings_.reset.GPIO_Pin,  GPIO_PIN_SET );
    HAL_Delay(500);
}

void wifi::detail::WifiSPI::enable_nss()
{
	HAL_GPIO_WritePin(io_settings_.nss.GPIOx, io_settings_.nss.GPIO_Pin, GPIO_PIN_RESET);
	HAL_Delay(10);
}

void wifi::detail::WifiSPI::disable_nss()
{
	HAL_GPIO_WritePin(io_settings_.nss.GPIOx, io_settings_.nss.GPIO_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
}

size_t wifi::detail::WifiSPI::receive(char* data, uint16_t len, uint32_t timeout)
{
	disable_nss();
	while(!is_cmddata_ready()){}
	enable_nss();
	int16_t length = 0;

	while(is_cmddata_ready())
	{
		if((length < len) || !len)
		{
			HAL_SPI_Receive(&io_settings_.data, reinterpret_cast<uint8_t*>(data), 1, timeout);
			length += 2;
			data += 2;

		}
		else
		{
			break;
		}
	}

	if(*(--data) == 0x15)
	{
		--length;
	}

	disable_nss();

	return length;
}

size_t wifi::detail::WifiSPI::send(const char* const data, uint16_t len, uint32_t timeout)
{
	char* payload;
	while(!is_cmddata_ready()){}

	enable_nss();

	if(len & 0x1)
	{
		payload = new char[len+1];
		payload[len] = '\n';
	}
	else
	{
		payload = new char[len];
	}
	memcpy(payload, data, len);

	if(HAL_SPI_Transmit(&io_settings_.data, reinterpret_cast<uint8_t*>(payload), (len+1)/2, timeout) != HAL_OK)
	{
		return -1;
	}

	delete[] payload;
	return len;
}

bool wifi::detail::WifiSPI::is_cmddata_ready()
{
	return HAL_GPIO_ReadPin(io_settings_.data_ready.GPIOx, io_settings_.data_ready.GPIO_Pin) == GPIO_PIN_SET;
}

bool wifi::detail::WifiSPI::execute(char* cmd, char* data, uint32_t timeout)
{
	send(cmd, strlen((char*)cmd), timeout);
	int len = receive(data, 0, timeout);
	if(len > 0)
	{
		*(data + len) = 0;
		if(strstr((char*)data, "\r\nOK\r\n> "))
		{
			return true;
		}
	}
	return false;
}

size_t wifi::detail::WifiSPI::send(const char* const data, uint32_t timeout)
{
	return send(data, strlen(data), timeout);
}
