#include "string.h"
#include "stdio.h"

void sent_string_to_mcu(char data[]){
	char stringBuffer[500];
	sprintf(stringBuffer, "%s\n" , data);
	HAL_UART_Transmit(&huart4, (uint8_t*) stringBuffer, strlen(stringBuffer), 200);
}
