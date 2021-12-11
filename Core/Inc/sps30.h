#include "stdbool.h"
#include "string.h"
#include "stdio.h"

// Pre-define frame payload
uint8_t start[] = {0x00, 0x00, 0x7E, 0x00, 0x00, 0x02, 0x01, 0x03, 0xF9, 0x7E};
uint8_t read[] = {0x00, 0x00, 0x7E, 0x00, 0x03, 0x00, 0xFC, 0x7E};
uint8_t stop[] = {0x00, 0x00, 0x7E, 0x00, 0x01, 0x00, 0xFE, 0x7E};
uint8_t fan[] = {0x00, 0x00, 0x7E, 0x00, 0x56, 0x00, 0xA9, 0x7E};
uint8_t reset[] = {0x00, 0x00, 0x7E, 0x00, 0xD3, 0x00, 0x2C, 0x7E};
uint8_t ProductName[] = {0x00, 0x00, 0x7E, 0x00, 0xD0, 0x01, 0x01, 0x2D, 0x7E};
uint8_t ArticleCode[] = {0x00, 0x00, 0x7E, 0x00, 0xD0, 0x01, 0x02, 0x2C, 0x7E};
uint8_t SerialNumber[] = {0x00, 0x00, 0x7E, 0x00, 0xD0, 0x01, 0x03, 0x2B, 0x7E};
static int pre_define_data_size = 70;

// Utility function
void println(char input[])
{
	char stringBuffer[30];
	sprintf(stringBuffer, "%s\r\n", input);
	HAL_UART_Transmit(&huart3, (uint8_t *)stringBuffer, strlen(stringBuffer), 200);
}

void print_whole_data_array(uint8_t data[])
{
	while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET)
	{
	}
	HAL_UART_Transmit(&huart3, (uint8_t *)data, pre_define_data_size, 1000);
}

// For convert concatenate hex to float ieee754
float sensirion_bytes_to_float(const uint32_t bytes)
{
	union
	{
		uint32_t u32_value;
		float float32;
	} tmp;

	tmp.u32_value = bytes;
	return tmp.float32;
}

// Wake up SPS30 by sent start frame
uint8_t *wake_sensirion()
{
	// ! For debug only
	//println("Sent start frame");

	static uint8_t data[70];
	// Populate an array
	for (int i = 0; i < sizeof(data); i++)
	{
		data[i] = 0x00;
	}
	while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET)
	{
	}
	HAL_UART_Transmit(&huart1, (uint8_t *)start, sizeof(start), 1000);
	while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET)
	{
	}
	HAL_UART_Receive(&huart1, (uint8_t *)data, sizeof(data), 1000);
	//HAL_Delay(1000);
	return data;
}

// Read from SPS30 by sent read frame
float *read_sensirion()
{
	// ! For debug only
	//println("Sent read frame");

	uint8_t data[70];
	// Populate an array
	for (int i = 0; i < sizeof(data); i++)
	{
		data[i] = 0x00;
	}
	while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET)
	{
	}
	HAL_UART_Transmit(&huart1, (uint8_t *)read, sizeof(read), 1000);
	while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET)
	{
	}
	HAL_UART_Receive(&huart1, (uint8_t *)data, sizeof(data), 100); // from 1000 to 100

	// Check for start frame
	if (data[0] == 0x7E && data[1] == 0x00)
	{
		// Header frame
		uint8_t command = data[2];
		uint8_t errorcode = data[3];
		uint8_t length = data[4];
		uint8_t state = data[5];

		uint8_t checksum;
		// Find checksum frame by start from the back of the array
		for (uint8_t i = sizeof(data); i > 0; i--)
		{
			if (data[i] == 0x7E)
			{
				data[i - 1] = checksum;
			}
		}

		uint32_t concatenateHex[10];
		static float actualValue[10];
		// Concatenate HEX(2,2,2,2) into HEX(8) and convert concatenate hex to float ieee754
		for (int i = 0; i < 10; i++)
		{
			// 40 HEX(2) into 10 Groups
			// Skip first 5 bits (header)
			concatenateHex[i] = ((data[4 * i + 5]) << 24) + ((data[(4 * i) + 1 + 5]) << 16) + ((data[(4 * i) + 2 + 5]) << 8) + (data[(4 * i) + 3 + 5]);

			// Convert sensirion bytes to float
			actualValue[i] = sensirion_bytes_to_float(concatenateHex[i]);

			// ! For debug only
			char stringBuffer[30];
			sprintf(stringBuffer, "%.4f\r\n", actualValue[i]);
			HAL_UART_Transmit(&huart3, (uint8_t *)stringBuffer, strlen(stringBuffer), 200);
		}
		//HAL_Delay(1000);
		return actualValue;
	}
	else
	{
		//HAL_Delay(1000);
		return NULL;
	}
}
