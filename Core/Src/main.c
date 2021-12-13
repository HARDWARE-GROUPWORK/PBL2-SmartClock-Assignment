/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
#include "ILI9341_Touchscreen.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "snow_tiger.h"
#include "am2320.h"
#include "stdbool.h"
#include "sps30.h"
#include "uart_to_mcu.h"

#include "EEPROM.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

//Temperature and Humid
int lightPercent = 100;
float temp = 99.9;
float humid = 99.9;
float pmTwoPointFive = 40.0; // ug/m^3
float prevTemp = -1.0;
float prevHumid = -1.0;
float prevPmTwoPointFive = -1.0;

//Alarm
int32_t alarmMinute = 0;
int32_t alarmHour = 0;

//Prev Clock Timer
int32_t prevAlarmMinute = -1;
int32_t prevAlarmHour = -1;

//Clock Timer
uint32_t millisecond = 0;
uint32_t millisecondStopWatch = 0;
uint32_t halfsecond = 0;

int32_t secondNum = 50;
int32_t minuteNum = 37;
int32_t hourNum = 12;

//Prev Clock Timer
int32_t prevSecondNum = -1;
int32_t prevMinuteNum = -1;
int32_t prevHourNum = -1;

//ADC
volatile uint32_t adc_val = 0;

//Timer States
bool halfsecondState = true;

//Text
char Temp_Buffer_text[100];

//Horizontal Clock Screen
uint16_t maxWidth = 200; //300 //Left 50 - right 50
uint16_t offsetWidth = 60;
uint16_t maxHeight = 240;

//Horizontal Date Clock Screen
uint16_t offsetWidthDate = 40;

//Main States
int16_t mode = 0;
int16_t modeEdit = 1;
int16_t prevMode = -1;
int16_t prevModeEdit = -1;

//Button states
bool pressButton1 = false;
bool pressButton2 = false;
bool pressButton3 = false;
bool pressButton4 = false;

bool isPressButton1 = false;
bool isPressButton2 = false;
bool isPressButton3 = false;
bool isPressButton4 = false;

bool userResetButton = false;

//Pm state
bool isPmSend = false;

//Alarm State
bool alarmIsOn = true;
bool alarmIsAlert = false;

//HAL Timers
uint64_t secondCounter = 0;
uint64_t prevSecondCounter = 0;
uint64_t millisecondHAL = 0;

uint64_t pmPrevMillisecondHAL = 0;
uint64_t pmSendPrevMillisecondHAL = 0;
uint64_t buzzerPrevMillisecondHAL = 0;
uint64_t alarmPrevMillisecondHAL = 0;
bool buzzerIsOn = false;
uint8_t alarmNumOfAlert = 0;

//Date Clock
int8_t date = 13; // 1-31
char* dayText[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"}; // 7 day names
char* monthText[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"}; // 12 months    0 2 4 6 7 9 11 for 31 days
int8_t dayIndex = 1; // 0-6 only
int8_t monthIndex = 11; // 0-11 only
int16_t year = 2021; // year

//Prev Date Clock
int8_t prevDate = -1; // 1-31
int8_t prevDayIndex = -1; // 7 index day names
int8_t prevMonthIndex = -1; // 12  index months
int16_t prevYear = -1; // year

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
uint16_t CRC16_2(uint8_t *, uint8_t);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//Buzzer Sound
void buzzerSound(uint32_t delay){

	if(alarmIsAlert == false){
		if(buzzerIsOn == true){
	//		char hexString[30];
	//		sprintf(hexString,"BUZZER....\r\n");
	//		HAL_UART_Transmit(&huart3, (uint8_t*) hexString, strlen(hexString), 1000);

			htim4.Instance->CCR1 = (1000 - 1) * 0.5;
			HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
		}

		if(buzzerIsOn == true && millisecondHAL - buzzerPrevMillisecondHAL >= delay){
	//		char hexString[30];
	//		sprintf(hexString,"QUIT BUZZER\r\n");
	//		HAL_UART_Transmit(&huart3, (uint8_t*) hexString, strlen(hexString), 1000);

			HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
			buzzerIsOn = false;
			buzzerPrevMillisecondHAL = millisecondHAL;

		}
	}else{

		delay += 90;

		if(alarmNumOfAlert >= 60){
			alarmNumOfAlert = 0;
			alarmIsAlert = false;
		}

		if(alarmNumOfAlert % 4 == 0){
			delay += delay*2;
		}

		if(buzzerIsOn == false && millisecondHAL - buzzerPrevMillisecondHAL >= delay && alarmNumOfAlert < 60){
			buzzerIsOn = true;
			buzzerPrevMillisecondHAL = millisecondHAL;
			alarmNumOfAlert++;
		}


		if(buzzerIsOn == true){
//			char hexString[30];
//			sprintf(hexString,"BUZZER....\r\n");
//			HAL_UART_Transmit(&huart3, (uint8_t*) hexString, strlen(hexString), 1000);

			htim4.Instance->CCR1 = (1000 - 1) * 0.5;
			HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

		}



		if(buzzerIsOn == true && millisecondHAL - buzzerPrevMillisecondHAL >= delay){
//			char hexString[30];
//			sprintf(hexString,"QUIT BUZZER\r\n");
//			HAL_UART_Transmit(&huart3, (uint8_t*) hexString, strlen(hexString), 1000);

			HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
			buzzerIsOn = false;
			buzzerPrevMillisecondHAL = millisecondHAL;

		}

	}

}

// Save EEPROM
void saveData(){
	//Clock
	EEPROM_Write_NUM (1, 0, hourNum);
	EEPROM_Write_NUM (2, 0, minuteNum);
	EEPROM_Write_NUM (3, 0, secondNum);

	//Date
	EEPROM_Write_NUM (4, 0, dayIndex);
	EEPROM_Write_NUM (5, 0, date);
	EEPROM_Write_NUM (6, 0, monthIndex);
	EEPROM_Write_NUM (7, 0, year);

	//Alarm
	EEPROM_Write_NUM (8, 0, alarmHour);
	EEPROM_Write_NUM (9, 0, alarmMinute);
	EEPROM_Write_NUM (10, 0, alarmIsOn);
}
// Read EEPROM
void readData(){
	//Clock
	hourNum = EEPROM_Read_NUM (1, 0);
	minuteNum = EEPROM_Read_NUM (2, 0);
	secondNum = EEPROM_Read_NUM (3, 0);

	//Date
	dayIndex = EEPROM_Read_NUM (4, 0);
	date = EEPROM_Read_NUM (5, 0);
	monthIndex = EEPROM_Read_NUM (6, 0);
	year = EEPROM_Read_NUM (7, 0);

	//Alarm
	alarmHour = EEPROM_Read_NUM (8, 0);
	alarmMinute = EEPROM_Read_NUM (9, 0);
	alarmIsOn = EEPROM_Read_NUM (10, 0);
}

// Erase EERPOM
void eraseAllData(){
	for (int i=0; i<512; i++)
	{
	  EEPROM_PageErase(i);
	}
	year = 2021;
	EEPROM_Write_NUM (7, 0, year); // override year
	readData(); // Read from Clean EEPROM
}

void checkResetData(){
	if(userResetButton == 1){
		eraseAllData();
		userResetButton = 0;
	}
}

// Paint screen black
void setHorizontalScreen(uint16_t color){
	ILI9341_Fill_Screen(color);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
}

bool check31Days(){
	if(monthIndex == 0 || monthIndex == 2 || monthIndex == 4 || monthIndex == 6 || monthIndex == 7 || monthIndex == 9 || monthIndex == 11){
		return true;
	}else{
		return false;
	}
}
void setDayOneIncrementMonth(uint8_t num){
	if(date > num){
		date = 1;
		if(mode != 100){
			monthIndex++;
		}
	}
}
void setDayX(uint8_t num){
	date = num;
}

void compareAlarmClock(){ //Check If alarmIsOn and equal to alarm setting, alert!
//	char hexString[30];
//	sprintf(hexString,"%d %d %d // %d %d\r\n",hourNum,minuteNum,secondNum,alarmHour,alarmMinute);
//	HAL_UART_Transmit(&huart3, (uint8_t*) hexString, strlen(hexString), 1000);
	if(alarmIsOn == true && hourNum == alarmHour && minuteNum == alarmMinute && secondNum == 0){
		alarmIsAlert = true;
		char hexString[30];
		sprintf(hexString,"Clock Alert! First Time\r\n");
		HAL_UART_Transmit(&huart3, (uint8_t*) hexString, strlen(hexString), 1000);
	}
}

void alarmClockSound(){


}

//Calculation
void calculationClock(){

	millisecondHAL = HAL_GetTick();

//	char hexString[30];
//	sprintf(hexString,"%d\r\n", millisecond);
//	HAL_UART_Transmit(&huart3, (uint8_t*) hexString, strlen(hexString), 1000);

	//Normal Clock
	if (millisecond >= 1000){
		millisecond = millisecond - 1000;
		secondNum++;
	}
	if (secondNum >= 60){
		secondNum = 0;
		if (mode != 100){
			minuteNum++;
		}
		compareAlarmClock();
	}
	if (minuteNum >= 60){
		minuteNum = 0;
		if (mode != 100){
			hourNum++;
		}
	}
	if (hourNum >= 24){
		hourNum = 0;
		if (mode != 100){
			dayIndex++;
			date++;
		}
	}
	//Normal Date
	if (dayIndex >= 7){
		dayIndex = 0;
	}
	if (year % 4 == 0){ //check for FEB 29 days
		if(monthIndex == 1){
			setDayOneIncrementMonth(29); // 29 days
		}else if(check31Days() == true){
			setDayOneIncrementMonth(31); // 31 days
		}else{
			setDayOneIncrementMonth(30); // 30 days
		}
	}else{
		if(monthIndex == 1){
			setDayOneIncrementMonth(28); // 28 days
		}else if(check31Days() == true){
			setDayOneIncrementMonth(31); // 31 days
		}else{
			setDayOneIncrementMonth(30); // 30 days
		}
	}
	if (monthIndex >= 12){
		monthIndex = 0;
		if (mode != 100){
			year++;
		}
	}
	if (year >= 10000){
		year = 1;
	}

	//check for editMode
	if(mode == 100){
		if (minuteNum < 0){
			minuteNum = 59;
		}
		if (hourNum < 0){
			hourNum = 23;
		}
		if (dayIndex < 0){
			dayIndex = 6;
		}
		if (date < 1){
			if (year % 4 == 0){ //check for FEB 29 days
				if(monthIndex == 1){
					setDayX(29); // 29 days
				}else if(check31Days() == true){
					setDayX(31); // 31 days
				}else{
					setDayX(30); // 30 days
				}
			}else{
				if(monthIndex == 1){
					setDayX(28); // 28 days
				}else if(check31Days() == true){
					setDayX(31); // 31 days
				}else{
					setDayX(30); // 30 days
				}
			}
		}
		if (monthIndex < 0){
			monthIndex = 11;
		}
		if (year < 1){
			year = 9999;
		}
	}

	saveData();
}

//Date Clock Atomic
void dayScreen(bool status, bool isEdit){

	if (prevDayIndex != dayIndex || isEdit == true){
		if (status == true){
			sprintf(Temp_Buffer_text, "%s", dayText[dayIndex]);
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*0 -5, maxHeight * 0.1, WHITE, 2, BLACK);
		}
		else{
			sprintf(Temp_Buffer_text, "   ");
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*0 -5, maxHeight * 0.1, WHITE, 2, BLACK);
		}
		prevDayIndex = dayIndex;
	}
}
void dateScreen(bool status, bool isEdit){
	if (prevDate != date || isEdit == true){
		if (status == true){
			sprintf(Temp_Buffer_text, "%02d", (int)date);
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*1+8 -5, maxHeight * 0.1, WHITE, 2, BLACK);
		}
		else{
			sprintf(Temp_Buffer_text, "  ");
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*1+8 -5, maxHeight * 0.1, WHITE, 2, BLACK);
		}
		prevDate = date;
	}
}
void monthScreen(bool status, bool isEdit){
	if (prevMonthIndex != monthIndex || isEdit == true){
		if (status == true){
			sprintf(Temp_Buffer_text, "%s", monthText[monthIndex]);
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*2 -5, maxHeight * 0.1, WHITE, 2, BLACK);
		}
		else{
			sprintf(Temp_Buffer_text, "   ");
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*2 -5, maxHeight * 0.1, WHITE, 2, BLACK);
		}
		prevMonthIndex = monthIndex;
	}
}
void yearScreen(bool status, bool isEdit){
	if (prevYear != year || isEdit == true){
		if (status == true){
			sprintf(Temp_Buffer_text, "%04d", (int)year);
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*3+8 -5, maxHeight * 0.1, WHITE, 2, BLACK);
		}
		else{
			sprintf(Temp_Buffer_text, "    ");
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*3+8 -5, maxHeight * 0.1, WHITE, 2, BLACK);
		}
		prevYear = year;
	}
}

//Display Date Clock Screen
void displayDateScreen(){
	dayScreen(true, false);
	dateScreen(true, false);
	monthScreen(true, false);
	yearScreen(true, false);
}

void displayAlarmIcon(bool on){
	if(on == true){
		//Alarm
		ILI9341_Draw_Filled_Circle(maxWidth * 0.92 + offsetWidth, maxHeight * 0.13, 6, YELLOW);
		ILI9341_Draw_Filled_Circle(maxWidth * 0.92 + offsetWidth, maxHeight * 0.16-1, 3, YELLOW);
		ILI9341_Draw_Filled_Rectangle_Coord(maxWidth*0.9 +offsetWidth-6, maxHeight * 0.1 +9, maxWidth*0.9 +offsetWidth+14, maxHeight * 0.1 +13, YELLOW);
		ILI9341_Draw_Hollow_Rectangle_Coord(maxWidth*0.9 +offsetWidth-6, maxHeight * 0.1 +9, maxWidth*0.9 +offsetWidth+14, maxHeight * 0.1 +13, BLACK);
	}else{
		//Alarm (Black Icon)
		ILI9341_Draw_Filled_Circle(maxWidth * 0.92 + offsetWidth, maxHeight * 0.13, 6, BLACK);
		ILI9341_Draw_Filled_Circle(maxWidth * 0.92 + offsetWidth, maxHeight * 0.16-1, 3, BLACK);
		ILI9341_Draw_Filled_Rectangle_Coord(maxWidth*0.9 +offsetWidth-6, maxHeight * 0.1 +9, maxWidth*0.9 +offsetWidth+14, maxHeight * 0.1 +13, BLACK);
		ILI9341_Draw_Hollow_Rectangle_Coord(maxWidth*0.9 +offsetWidth-6, maxHeight * 0.1 +9, maxWidth*0.9 +offsetWidth+14, maxHeight * 0.1 +13, BLACK);
	}
}

//Top Screen
void topBarScreen(){
	displayDateScreen();

	displayAlarmIcon(alarmIsOn);
}

//Reset Prev Values
void resetPrevNum(){
	prevSecondNum = -1;
	prevMinuteNum = -1;
	prevHourNum = -1;

	prevDayIndex = -1;
	prevDate = -1;
	prevMonthIndex = -1;
	prevYear = -1;

	prevTemp = -1.0;
	prevHumid = -1.0;
	prevPmTwoPointFive = -1.0;
}

//Clock Screen Atomic
void hourScreen(bool status, bool isEdit){
	if (prevHourNum != hourNum || isEdit == true){
		if (status == true){
			sprintf(Temp_Buffer_text, "%02d", (int)hourNum);
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth - 5, maxHeight * 0.3, WHITE, 6, BLACK);
		}
		else{
			sprintf(Temp_Buffer_text, "  ");
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth - 5, maxHeight * 0.3, WHITE, 6, BLACK);
		}
		prevHourNum = hourNum;
	}
}

void colonScreen(bool status){
	if (status == true){
		sprintf(Temp_Buffer_text, ":");
		ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth + 73, maxHeight * 0.35, WHITE, 4, BLACK);
	}
	else{
		sprintf(Temp_Buffer_text, " ");
		ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth + 73, maxHeight * 0.35, WHITE, 4, BLACK);
	}
}
void minuteScreen(bool status, bool isEdit){
	if (prevMinuteNum != minuteNum || isEdit == true){
		if (status == true){

			sprintf(Temp_Buffer_text, "%02d", (int)minuteNum);
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth + 97, maxHeight * 0.3, WHITE, 6, BLACK);
		}
		else{
			sprintf(Temp_Buffer_text, "  ");
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth + 97, maxHeight * 0.3, WHITE, 6, BLACK);
		}
		prevMinuteNum = minuteNum;
	}
}
void secondScreen(bool status, bool isEdit){
	if (prevSecondNum != secondNum || isEdit == true){
		if (status == true){
			sprintf(Temp_Buffer_text, "%02d", (int)secondNum);
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0.90 + offsetWidth -3, maxHeight * 0.42, WHITE, 2, BLACK);
		}
		else{
			sprintf(Temp_Buffer_text, "  ");
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0.90 + offsetWidth -3, maxHeight * 0.42, WHITE, 2, BLACK);
		}
		prevSecondNum = secondNum;
	}
}

//Display Clock Screen
void displayClockScreen(){

	if (halfsecondState == false){ // colon behaviour
		colonScreen(true);
	}
	else{
		colonScreen(false);
	}

	secondScreen(true, false);
	minuteScreen(true, false);
	hourScreen(true, false);
}

void staticClockScreen(){
	colonScreen(true);
	hourScreen(true, false);
	minuteScreen(true, false);
	secondScreen(true, false);
}

//Edit Clock Screen
void editHourScreen()
{
	displayDateScreen(); // Init first
	colonScreen(true);
	minuteScreen(true, false);
	secondScreen(true, false);

	if (halfsecondState == false){ // hour
		hourScreen(false, true);
	}
	else{
		hourScreen(true, true);
	}
}
void editMinuteScreen(){

	colonScreen(true);
	hourScreen(true, false);
	secondScreen(true, false);

	if (halfsecondState == false){ //minute
		minuteScreen(false, true);
	}
	else{
		minuteScreen(true, true);
	}
}
void editSecondScreen(){

	colonScreen(true);
	hourScreen(true, false);
	minuteScreen(true, false);

	if (halfsecondState == false){ //second
		secondScreen(false, true);
	}
	else{
		secondScreen(true, true);
	}
}

//Edit Date Clock Screen
void editDayScreen()
{
	dateScreen(true, false);
	monthScreen(true, false);
	yearScreen(true, false);

	if (halfsecondState == false){ // day
		dayScreen(false, true);
	}
	else{
		dayScreen(true, true);
	}
}
void editDateScreen()
{
	dayScreen(true, false);
	monthScreen(true, false);
	yearScreen(true, false);

	if (halfsecondState == false){ // date
		dateScreen(false, true);
	}
	else{
		dateScreen(true, true);
	}
}
void editMonthScreen()
{
	dayScreen(true, false);
	dateScreen(true, false);
	yearScreen(true, false);

	if (halfsecondState == false){ // month
		monthScreen(false, true);
	}
	else{
		monthScreen(true, true);
	}
}
void editYearScreen()
{
	staticClockScreen(); // init Clock first in editing mode
	dayScreen(true, false);
	dateScreen(true, false);
	monthScreen(true, false);

	if (halfsecondState == false){ // day
		yearScreen(false, true);
	}
	else{
		yearScreen(true, true);
	}
}

//Bottom Screen
void bottomBarScreen(){

	uint8_t size = 2;
	uint8_t bottomHeight = maxHeight * 0.87;
	uint8_t bottomWidth1 = maxWidth * 0 + 51;
	uint8_t bottomWidth2 = maxWidth * 0.25 + 51;
	uint8_t bottomWidth3 = maxWidth * 0.50 + 51;
	uint8_t bottomWidth4 = maxWidth * 0.75 + 51;
	uint8_t bottomWidth = 55;

	//Rectangle Background Color
	ILI9341_Draw_Filled_Rectangle_Coord(bottomWidth1, bottomHeight, bottomWidth1 + bottomWidth, maxHeight, RED);
	ILI9341_Draw_Filled_Rectangle_Coord(bottomWidth2, bottomHeight, bottomWidth2 + bottomWidth, maxHeight, YELLOW);
	ILI9341_Draw_Filled_Rectangle_Coord(bottomWidth3, bottomHeight, bottomWidth3 + bottomWidth, maxHeight, CYAN);
	ILI9341_Draw_Filled_Rectangle_Coord(bottomWidth4, bottomHeight, bottomWidth4 + bottomWidth, maxHeight, GREEN);

	//Text Layout
	sprintf(Temp_Buffer_text, "MOD");
	ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth, maxHeight * 0.9, BLACK, size, RED);
	sprintf(Temp_Buffer_text, "ADJ");
	ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0.25 + offsetWidth, maxHeight * 0.9, BLACK, size, YELLOW);
	sprintf(Temp_Buffer_text, "BWD");
	ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0.5 + offsetWidth, maxHeight * 0.9, BLACK, size, CYAN);
	sprintf(Temp_Buffer_text, "FWD");
	ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0.75 + offsetWidth, maxHeight * 0.9, BLACK, size, GREEN);
}
void bottomBarScreenUpdate(){

	uint8_t size = 2;
	uint8_t bottomWidth1 = maxWidth * 0 + 51;
	uint8_t bottomHeight = maxHeight * 0.87;
	uint8_t bottomWidth4 = maxWidth * 0.75 + 51;
	uint8_t bottomWidth = 55;

	//Update Temperature
	if(prevTemp != temp){
		sprintf(Temp_Buffer_text, "%0.1f 'C", temp);
		ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0.0 + offsetWidth, maxHeight * 0.7-10, WHITE, size, BLACK);
		prevTemp = temp;
		ILI9341_Draw_Hollow_Rectangle_Coord(bottomWidth1, bottomHeight-45-10, bottomWidth4 + bottomWidth, maxHeight-52-10, WHITE);
	}
	//Update Humidity
	if(prevHumid != humid){
		sprintf(Temp_Buffer_text, "%0.1f %%", humid);
		ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0.55 + offsetWidth, maxHeight * 0.7-10, WHITE, size, BLACK);
		prevHumid = humid;
		ILI9341_Draw_Hollow_Rectangle_Coord(bottomWidth1, bottomHeight-45-10, bottomWidth4 + bottomWidth, maxHeight-52-10, WHITE);
	}
	//Update PM2.5
	if(prevPmTwoPointFive != pmTwoPointFive){ // update this value please
		sprintf(Temp_Buffer_text, "      %03d ug/m^3", (int)pmTwoPointFive);
		ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0.0 + offsetWidth, maxHeight * 0.7+18, WHITE, size, BLACK); // Change color

		sprintf(Temp_Buffer_text, "PM2.5");
		ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0.0 + offsetWidth, maxHeight * 0.7+18, WHITE, size, BLACK);

		prevPmTwoPointFive = pmTwoPointFive;
		ILI9341_Draw_Hollow_Rectangle_Coord(bottomWidth1, bottomHeight-45+18, bottomWidth4 + bottomWidth, maxHeight-52+18, WHITE);
	}
}

void notifyPm(){
	float *respondRead;
		respondRead = read_sensirion();

	if(respondRead[1] >= 999){
		respondRead[1] = 999;
	}
	if(respondRead[2] >= 999){
		respondRead[2] = 999;
	}
	if(respondRead[3] >= 999){
		respondRead[3] = 999;
	}
	if(respondRead[4] >= 999){
		respondRead[4] = 999;
	}

	if(respondRead[1] > 0 && respondRead[1] <= 9999 &&
		respondRead[2] > 0 && respondRead[2] <= 9999 &&
		respondRead[3] > 0 && respondRead[3] <= 9999 &&
		respondRead[4] > 0 && respondRead[4] <= 9999){

		pmTwoPointFive = respondRead[1];

		if(isPmSend == false){
			if(respondRead[1]>=250){
				sent_string_to_mcu("HAZ");
			}
			else if(respondRead[1]>=150){
				sent_string_to_mcu("VUH");
			}
			else if(respondRead[1]>=55){
				sent_string_to_mcu("UHT");
			}
			if(respondRead[1]>=55){
				println("Danger Air");
				println("Sending");
				char stringBuffer[500];
				sprintf(stringBuffer, "EXC %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f" , respondRead[0], respondRead[1], respondRead[2], respondRead[3], respondRead[4], respondRead[5], respondRead[6], respondRead[7], respondRead[8], respondRead[9]);
				sent_string_to_mcu(stringBuffer);

				isPmSend = true;
				pmSendPrevMillisecondHAL = millisecondHAL;
			}
			else{
				println("Normal Air");
			}
		}

	}

	if(millisecondHAL - pmSendPrevMillisecondHAL >= 10000){
		isPmSend = false;
		pmSendPrevMillisecondHAL = millisecondHAL;
	}

//	float* respondRead;
//
//	println("zzz");
//	if(millisecondHAL - pmPrevMillisecondHAL >= 1000){
//		pmPrevMillisecondHAL = millisecondHAL;
//
//		println("xxxx");
//		respondRead = read_sensirion();
//
//		println("aaaaaaaaa");
//	}
//	println("HAHAHAHA");
//
//	if(respondRead[1] >= 0 && respondRead[1] <= 9999 &&
//		respondRead[2] >= 0 && respondRead[2] <= 9999 &&
//		respondRead[3] >= 0 && respondRead[3] <= 9999 &&
//		respondRead[4] >= 0 && respondRead[4] <= 9999
//	  ){
//
//		pmTwoPointFive = respondRead[1];
//
//		if(isPmSend == false){
//			if(respondRead[1]>=250){
//				sent_string_to_mcu("HAZ");
//			}
//			else if(respondRead[1]>=150){
//				sent_string_to_mcu("VUH");
//			}
//			else if(respondRead[1]>=55){
//				sent_string_to_mcu("UHT");
//			}
//			if(respondRead[1]>=55){
//				println("Danger Air");
//				println("Sending");
//				char stringBuffer[500];
//				sprintf(stringBuffer, "EXC %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f" , respondRead[0], respondRead[1], respondRead[2], respondRead[3], respondRead[4], respondRead[5], respondRead[6], respondRead[7], respondRead[8], respondRead[9]);
//				sent_string_to_mcu(stringBuffer);
//
//				isPmSend = true;
//				pmSendPrevMillisecondHAL = millisecondHAL;
//			}
//			else{
//				println("Normal Air");
//			}
//		}
//	}
//
//	println("WTFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
//
//	if(millisecondHAL - pmSendPrevMillisecondHAL >= 10000){
//		isPmSend = false;
//		pmSendPrevMillisecondHAL = millisecondHAL;
//	}
}

void resisterMonitor(){

	  float dutyCycleScreen = 0.0;
	  while(HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK){}
	  adc_val = HAL_ADC_GetValue(&hadc1);
	  lightPercent = adc_val*100 / 4095;

	  //Change Screen Light Output
	  //PWM
	  dutyCycleScreen = ((adc_val/4095.0) * 0.8) + 0.2;
	  //No. 2
	  htim3.Instance -> CCR1 = (100-1) * dutyCycleScreen;

	  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}


int32_t stopWatchHour = 0;
int32_t stopWatchMinute = 0;
int32_t stopWatchSecond = 0;
int32_t stopWatchMillisecond = 0;
int32_t prevStopWatchHour = -1;
int32_t prevStopWatchMinute = -1;
int32_t prevStopWatchSecond = -1;
int32_t prevStopWatchMillisecond = -1;

//State
bool initialStopWatchScreen = false;
bool isStopWatchRunning = false;


void resetPrevStopWatch(){
	prevStopWatchHour = -1;
	prevStopWatchMinute = -1;
	prevStopWatchSecond = -1;
	prevStopWatchMillisecond = -1;
}
void resetStopWatch(){
	stopWatchHour = 0;
	stopWatchMinute = 0;
	stopWatchSecond = 0;
	stopWatchMillisecond = 0;
	resetPrevStopWatch();
}

void displayStopWatchScreen(){
	if(stopWatchHour < 1){ // Normal StopWatch Mode
		if(prevStopWatchMinute != stopWatchMinute){
			sprintf(Temp_Buffer_text, "%02d", (int)stopWatchMinute);
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth, maxHeight * 0.35+16, WHITE, 5, BLACK);
			prevStopWatchMinute = stopWatchMinute;
		}
		if(prevStopWatchSecond != stopWatchSecond){
			sprintf(Temp_Buffer_text, "%02d", (int)stopWatchSecond);
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth+70, maxHeight * 0.35+16, WHITE, 5, BLACK);
			prevStopWatchSecond = stopWatchSecond;
		}
		if(prevStopWatchMillisecond != stopWatchMillisecond){
			sprintf(Temp_Buffer_text, "%02d", (int)((stopWatchMillisecond/10)%100));
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth+140, maxHeight * 0.35+16, WHITE, 5, BLACK);
			prevStopWatchMillisecond = stopWatchMillisecond;
		}
	}else if (stopWatchHour >= 1){ // counting more than equal to 1 hour
		if(prevStopWatchHour != stopWatchHour){
			sprintf(Temp_Buffer_text, "%02d", (int)stopWatchHour);
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth, maxHeight * 0.35+16, WHITE, 5, BLACK);
			prevStopWatchHour = stopWatchHour;
		}
		if(prevStopWatchMinute != stopWatchMinute){
			sprintf(Temp_Buffer_text, "%02d", (int)stopWatchMinute);
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth+70, maxHeight * 0.35+16, WHITE, 5, BLACK);
			prevStopWatchMinute = stopWatchMinute;
		}
		if(prevStopWatchSecond != stopWatchSecond){
			sprintf(Temp_Buffer_text, "%02d", (int)stopWatchSecond);
			ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth+140, maxHeight * 0.35+16, WHITE, 5, BLACK);
			prevStopWatchSecond = stopWatchSecond;
		}
	}
}

void stopWatchScreen(){

	if(initialStopWatchScreen == false){
		resetPrevStopWatch();
		//Statics
		displayAlarmIcon(alarmIsOn);

		sprintf(Temp_Buffer_text, "Stopwatch");
		ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*0, maxHeight * 0.1, WHITE, 2, BLACK);

		sprintf(Temp_Buffer_text, ":");
		ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth+60, maxHeight * 0.37+16, WHITE, 4, BLACK);
		sprintf(Temp_Buffer_text, ":");
		ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth+130, maxHeight * 0.37+16, WHITE, 4, BLACK);

		initialStopWatchScreen = true;
	}

	//Time running
	if(isStopWatchRunning == true){
		stopWatchMillisecond += millisecondStopWatch;
		millisecondStopWatch = 0;
	}

	//Normal Clock
	if (stopWatchMillisecond >= 1000){
		stopWatchMillisecond = stopWatchMillisecond - 1000; //Keep remainder of millisecondStopWatch
		stopWatchSecond++;
	}
	if (stopWatchSecond >= 60){
		stopWatchSecond = 0;
		stopWatchMinute++;
	}
	if (stopWatchMinute >= 60){
		stopWatchMinute = 0;
		stopWatchHour++;
	}
	if (stopWatchHour >= 99){
		stopWatchHour = 0;
	}

	//Running
	displayStopWatchScreen();


//	char hexString[30];
//	sprintf(hexString,"%d %d\r\n",prevStopWatchHour,stopWatchHour);
//	HAL_UART_Transmit(&huart3, (uint8_t*) hexString, strlen(hexString), 1000);

}


//State
bool initialAlarmClockScreen = false;
bool initialEditAlarmClockScreen = false;

void resetPrevAlarm(){
	prevAlarmMinute = -1;
	prevAlarmHour = -1;
}

void hourAlarmScreen(bool status, bool isEdit){
	if (prevAlarmHour != alarmHour || isEdit == true){
		if (status == true){
			sprintf(Temp_Buffer_text, "%02d", (int)alarmHour);
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth, maxHeight * 0.3 +10, WHITE, 6, BLACK);
		}
		else{
			sprintf(Temp_Buffer_text, "  ");
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth, maxHeight * 0.3 +10, WHITE, 6, BLACK);
		}
		prevAlarmHour = alarmHour;
	}
}

void minuteAlarmScreen(bool status, bool isEdit){
	if (prevAlarmMinute != alarmMinute || isEdit == true){
		if (status == true){

			sprintf(Temp_Buffer_text, "%02d", (int)alarmMinute);
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth + 110, maxHeight * 0.3 +10, WHITE, 6, BLACK);
		}
		else{
			sprintf(Temp_Buffer_text, "  ");
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth + 110, maxHeight * 0.3 +10, WHITE, 6, BLACK);
		}
		prevAlarmMinute = alarmMinute;
	}
}
void colonAlarmScreen(){
	sprintf(Temp_Buffer_text, ":");
	ILI9341_Draw_Text(Temp_Buffer_text, maxWidth * 0 + offsetWidth + 80, maxHeight * 0.35 +10, WHITE, 4, BLACK);
}

void alarmClockScreen(){
	if(initialAlarmClockScreen == false){
		resetPrevAlarm();

		//Statics
		sprintf(Temp_Buffer_text, "Alarm");
		ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*0, maxHeight * 0.1, WHITE, 2, BLACK);

		displayAlarmIcon(alarmIsOn);

		colonAlarmScreen();
		hourAlarmScreen(true,false);
		minuteAlarmScreen(true,false);

		initialAlarmClockScreen = true;
	}
}

void editScreen(){
	if(modeEdit == 1){
		editYearScreen();
	}else if (modeEdit == 2){
		editMonthScreen();
	}else if (modeEdit == 3){
		editDateScreen();
	}else if (modeEdit == 4){
		editDayScreen();
	}else if (modeEdit == 5){
		editHourScreen();
	}else if (modeEdit == 6){
		editMinuteScreen();
	}else if (modeEdit == 7){
		editSecondScreen();
	}
}
void editAlarmHourScreen(){

	minuteAlarmScreen(true,false);

	if (halfsecondState == false){ // Hour
		hourAlarmScreen(false, true);
	}
	else{
		hourAlarmScreen(true, true);
	}
}

void editAlarmMinuteScreen(){
	hourAlarmScreen(true,false);

	if (halfsecondState == false){ // Minute
		minuteAlarmScreen(false, true);
	}
	else{
		minuteAlarmScreen(true, true);
	}
}
void editAlarmScreen(){
	if(initialEditAlarmClockScreen == false){
		resetPrevAlarm();
		displayAlarmIcon(alarmIsOn);

		sprintf(Temp_Buffer_text, "Alarm");
		ILI9341_Draw_Text(Temp_Buffer_text, offsetWidth + offsetWidthDate*0, maxHeight * 0.1, WHITE, 2, BLACK);

		hourAlarmScreen(true,false);
		minuteAlarmScreen(true,false);

		initialEditAlarmClockScreen = true;
	}

	//Algorithm Calculation
	if (alarmMinute >= 60){
		alarmMinute = 0;
	}
	else if(alarmMinute < 0){
		alarmMinute = 59;
	}
	if (alarmHour >= 24){
		alarmHour = 0;
	}
	else if(alarmHour < 0){
		alarmHour = 23;
	}

	if(modeEdit == 1){
		editAlarmHourScreen();
	}else if (modeEdit == 2){
		editAlarmMinuteScreen();
	}
}


char str[50];
uint8_t cmdBuffer[3];
uint8_t dataBuffer[8];

void tempMonitor(){
	//Temperature
	cmdBuffer[0] = 0x03;
	cmdBuffer[1] = 0x00;
	cmdBuffer[2] = 0x04;

	//Send Temp & Humid via UART3
//	sprintf(str, "Temperature = %4.1f\tHumidity = %4.1f\n\r", temp, humid);
//	while(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC)==RESET){}
//	HAL_UART_Transmit(&huart3, (uint8_t*) str, strlen(str),200);

	//HAL_Delay(5000); //>3000 ms
	//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);

	//Wake up sensor
	HAL_I2C_Master_Transmit(&hi2c1, 0x5c<<1, cmdBuffer, 3, 200);
	//Send reading command
	HAL_I2C_Master_Transmit(&hi2c1, 0x5c<<1, cmdBuffer, 3, 200);

	HAL_Delay(80); // 50 is too low, 80 is okay

	//Receive sensor data
	HAL_I2C_Master_Receive(&hi2c1, 0x5c<<1, dataBuffer, 8, 200);

	uint16_t Rcrc = dataBuffer[7] << 8;
	Rcrc += dataBuffer[6];
	if (Rcrc == CRC16_2(dataBuffer, 6)) {
		uint16_t temperature = ((dataBuffer[4] & 0x7F) << 8) + dataBuffer[5];
		temp = temperature / 10.0;
		temp = (((dataBuffer[4] & 0x80) >> 7)== 1) ? (temp * (-1)) : temp ; // the temperature can be negative

		uint16_t humidity = (dataBuffer[2] << 8) + dataBuffer[3];
		humid = humidity / 10.0;
	}
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_SPI5_Init();
  MX_TIM1_Init();
  MX_RNG_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_UART4_Init();
  MX_ADC1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

	//Temp but not has code in here yet
	cmdBuffer[0] = 0x03;
	cmdBuffer[1] = 0x00;
	cmdBuffer[2] = 0x04;

	//initial driver setup to drive ili9341
	ILI9341_Init();

	//Interrupt millisecond
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim2);

	//Reset Screen
	setHorizontalScreen(BLACK);

	//Read EEPROM First Time
	readData();

	//ADC Input variable Resister(Light)
	HAL_ADC_Start(&hadc1);

	// Setup PM Sensor
	uint8_t* respondStart;
	respondStart = wake_sensirion();
	sent_string_to_mcu("STA");


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		// REAL CODE BEGIN



		calculationClock();
		checkResetData();
		resisterMonitor(); //light screen

		if (prevMode != mode || prevModeEdit != modeEdit){
			prevModeEdit = modeEdit;
			resetPrevNum();
			resetPrevAlarm();
		}
		// When Change Mode
		if (prevMode != mode){
			prevMode = mode;
			setHorizontalScreen(BLACK);
			bottomBarScreen();

			//For Mode 1 StopWatch
			initialStopWatchScreen = false;
			//For Mode 2 Alarm
			initialAlarmClockScreen = false;
			//For mode 200 Edit Alarm
			initialEditAlarmClockScreen = false;
		}

		if(mode == 0){
			if (halfsecond == 1){	// render every 500 ms
				halfsecondState = !halfsecondState; // check appearing of colon (:) in clock
				halfsecond = 0;

				tempMonitor(); // read every 500 ms
				notifyPm(); // read every 500 ms

				topBarScreen();
				displayClockScreen();
				bottomBarScreenUpdate();
			}
		}else if(mode == 1){	// No Notify Line at this mode because has delay
			stopWatchScreen();
		}else if(mode == 2){
			alarmClockScreen();
			if(halfsecond ==1){
				notifyPm(); // read every 500 ms
			}
		}else if (mode == 100){ // Adjust modeEdit 1-year, 2-month, 3-date, 4-day, 5-hour, 6-minute, 7-second

			if(halfsecond == 1){ // render every 500 ms
				halfsecondState = !halfsecondState; // check appearing of colon (:) in clock
				halfsecond = 0;

				notifyPm(); // read every 500 ms
				editScreen();
			}
		}else if (mode == 200){
			if(halfsecond == 1){ // render every 500 ms
				halfsecondState = !halfsecondState; // check appearing of colon (:) in clock
				halfsecond = 0;

				notifyPm(); // read every 500 ms
				editAlarmScreen();
			}
		}


		pressButton1 = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7); // pressButton1 is "true" when press, is "false" when not press
		pressButton2 = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6); // pressButton1 is "true" when press, is "false" when not press
		pressButton3 = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_5); // pressButton1 is "true" when press, is "false" when not press
		pressButton4 = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_4); // pressButton1 is "true" when press, is "false" when not press

		// NO BUTTON !!! THIS IS TEMPORARY
//		pressButton1 = 0;
//		pressButton2 = 0;
//		pressButton3 = 0;
//		pressButton4 = 0;

		//Buzzer
		if ((pressButton1 == true && isPressButton1 == false) ||
			(pressButton2 == true && isPressButton2 == false) ||
			(pressButton3 == true && isPressButton3 == false) ||
			(pressButton4 == true && isPressButton4 == false)){
			buzzerIsOn = true;
			buzzerPrevMillisecondHAL = millisecondHAL;
		}
		buzzerSound(40);

		//General Mode
		if (pressButton1 == true && isPressButton1 == false && !(mode == 100 || mode == 200)){ // increase mode only once
			mode++;
			if(mode == 3){
				mode = 0;
			}
			isPressButton1 = true;
			prevSecondCounter = millisecondHAL;
		}
		//StopWatch Mode
		if (pressButton2 == true && isPressButton2 == false && mode == 1 && isStopWatchRunning != true){ //Reset StopWatch
			isPressButton2 = true;
			resetStopWatch();
			prevSecondCounter = millisecondHAL;
		}
		if (pressButton3 == true && isPressButton3 == false && mode == 1){ //Running/Stop StopWatch
			isPressButton3 = true;
			if(isStopWatchRunning == false){ // press button
				millisecondStopWatch = 0;
				isStopWatchRunning = true;
			}else{
				isStopWatchRunning = false;
			}
			prevSecondCounter = millisecondHAL;
		}
		//Alarm Clock press when want to stop
		if((pressButton1|| pressButton2 || pressButton3 || pressButton4) && alarmIsAlert == true){
			isPressButton1 = true;
			isPressButton2 = true;
			isPressButton3 = true;
			isPressButton4 = true;
			prevSecondCounter = millisecondHAL;
			alarmIsAlert = false;
		}

		//Adjust Time Mode For Mode 0 and 100
		if (pressButton2 == true && isPressButton2 == false && mode == 0){ // initial time when pressButton2
			isPressButton2 = true;
			prevSecondCounter = millisecondHAL;
		}
		else if (pressButton2 == true && isPressButton2 == true && mode == 0 && millisecondHAL - prevSecondCounter >= 3000){ // hold for 3 seconds
			buzzerIsOn = true;
			buzzerPrevMillisecondHAL = millisecondHAL;
			buzzerSound(50);

			modeEdit = 1; // Reset to Year First time
			mode = 100;
			prevSecondCounter = millisecondHAL;
		}
		//Exit Adjust Time Mode
		if (pressButton2 == true && isPressButton2 == false && millisecondHAL - prevSecondCounter >= 1000 && mode == 100){
			isPressButton2 = true;
			modeEdit = 1;  // Reset to Year First time
			mode = 0;
			prevSecondCounter = millisecondHAL;
		}

		//Edit Mode
		if (pressButton1 == true && isPressButton1 == false && mode == 100){ // increase mode only once
			modeEdit++;
			isPressButton1 = true;
			if (modeEdit == 8){	 // finish loop edit
				modeEdit = 1; // Reset to Year
				mode = 0;	  // Back to General Mode
			}
			prevSecondCounter = millisecondHAL;
		}
		//Forward
		if (pressButton3 == true && isPressButton3 == false && mode == 100){ // increase value
			if (modeEdit == 1){
				year--;
			}else if (modeEdit == 2){
				monthIndex--;
			}else if (modeEdit == 3){
				date--;
			}else if (modeEdit == 4){
				dayIndex--;
			}else if (modeEdit == 5){
				hourNum--;
			}else if (modeEdit == 6){
				minuteNum--;
			}else if (modeEdit == 7){
				secondNum = 0;
			}
			halfsecondState = false;
			resetPrevNum();
			isPressButton3 = true;
			prevSecondCounter = millisecondHAL;
		}
		//Backward
		if (pressButton4 == true && isPressButton4 == false && mode == 100){ // decrease value
			if (modeEdit == 1){
				year++;
			}else if (modeEdit == 2){
				monthIndex++;
			}else if (modeEdit == 3){
				date++;
			}else if (modeEdit == 4){
				dayIndex++;
			}else if (modeEdit == 5){
				hourNum++;
			}else if (modeEdit == 6){
				minuteNum++;
			}else if (modeEdit == 7){
				secondNum = 0;
			}
			halfsecondState = false;
			resetPrevNum();
			isPressButton4 = true;
			prevSecondCounter = millisecondHAL;
		}



		//Adjust Alarm Mode For Mode 2 and 200
		if (pressButton2 == true && isPressButton2 == false && mode == 2){ // initial time when pressButton2
			isPressButton2 = true;
			alarmIsOn = !alarmIsOn;
			displayAlarmIcon(alarmIsOn);
			prevSecondCounter = millisecondHAL;
		}
		else if (pressButton2 == true && isPressButton2 == true && mode == 2 && millisecondHAL - prevSecondCounter >= 3000){ // hold for 3 seconds
			buzzerIsOn = true;
			buzzerPrevMillisecondHAL = millisecondHAL;
			buzzerSound(50);

			modeEdit = 1; // Reset to Hour First time
			mode = 200;
			alarmIsOn = true; // always on when editing this alarm
			prevSecondCounter = millisecondHAL;
		}
		//Exit Alarm Time Mode
		if (pressButton2 == true && isPressButton2 == false && millisecondHAL - prevSecondCounter >= 1000 && mode == 200){
			isPressButton2 = true;
			modeEdit = 1;  // Reset to Hour First time
			mode = 2;
			prevSecondCounter = millisecondHAL;
		}

		//Edit Mode
		if (pressButton1 == true && isPressButton1 == false && mode == 200){ // increase mode only once
			modeEdit++;
			isPressButton1 = true;
			if (modeEdit == 3){	 // finish loop edit
				modeEdit = 1; // Reset to Hour
				mode = 2;	  // Back to Alarm Mode
			}
			prevSecondCounter = millisecondHAL;
		}
		//Forward
		if (pressButton3 == true && isPressButton3 == false && mode == 200){ // increase value
			if (modeEdit == 1){
				alarmHour--;
			}else if (modeEdit == 2){
				alarmMinute--;
			}
			halfsecondState = false;
			resetPrevAlarm();
			isPressButton3 = true;
			prevSecondCounter = millisecondHAL;
		}
		//Backward
		if (pressButton4 == true && isPressButton4 == false && mode == 200){ // decrease value
			if (modeEdit == 1){
				alarmHour++;
			}else if (modeEdit == 2){
				alarmMinute++;
			}
			halfsecondState = false;
			resetPrevAlarm();
			isPressButton4 = true;
			prevSecondCounter = millisecondHAL;
		}



		if(millisecondHAL - prevSecondCounter >= 150){
			//Reset isPressButton
			if (pressButton1 == false){
				isPressButton1 = false;
			}
			if (pressButton2 == false){
				isPressButton2 = false;
			}
			if (pressButton3 == false){
				isPressButton3 = false;
			}
			if (pressButton4 == false){
				isPressButton4 = false;
			}
		}
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART3
                              |RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
uint16_t CRC16_2(uint8_t *ptr, uint8_t length){
	uint16_t crc = 0xFFFF;
	uint8_t s = 0x00;

	while (length--){
		crc ^= *ptr++;
		for (s = 0; s < 8; s++){
			if ((crc & 0x01) != 0){
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
				crc >>= 1;
		}
	}
	return crc;
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	//On Board
	//Blue
	if (GPIO_Pin == GPIO_PIN_13){
		userResetButton = 1;
		sprintf(str, "Interrupt pin13 \n\r");
		HAL_UART_Transmit(&huart3, (uint8_t*) str, strlen(str),200);
	}

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
