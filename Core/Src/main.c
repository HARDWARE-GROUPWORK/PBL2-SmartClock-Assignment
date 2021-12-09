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
int numberOfRecord = 1;
int lightPercent = 100;
float temp = 99.9;
float humid = 99.9;

uint32_t millisecond = 0;
uint32_t halfsecond = 0;

uint32_t secondNum = 50;
uint32_t minuteNum = 37;
uint32_t hourNum = 12;

uint32_t prevSecondNum = -1;
uint32_t prevMinuteNum = -1;
uint32_t prevHourNum = -1;

bool halfsecondState = true;
bool initialState = false;
char Temp_Buffer_text[40];

//Horizontal Screen
uint16_t maxWidth = 200; //300 //Left 50 - right 50
uint16_t offsetWidth = 60;
uint16_t maxHeight = 240;

uint16_t mode = 0;
uint16_t modeEdit = 1;
uint16_t prevMode = -1;
uint16_t prevModeEdit = -1;

bool pressButton1 = false;
bool pressButton2 = false;
bool pressButton3 = false;
bool pressButton4 = false;

bool isPressButton1 = false;
bool isPressButton2 = false;
bool isPressButton3 = false;
bool isPressButton4 = false;

uint16_t secondCounter = 0;
uint16_t prevSecondCounter = 0;
uint32_t millisecondHAL = 0;

char day[7][3] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
char month[12][3] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
uint16_t CRC16_2(uint8_t *, uint8_t );
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Paint screen black
void setHorizontalScreen(uint16_t color){
	ILI9341_Fill_Screen(color);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
}


void calculationClock(uint32_t ms){

	millisecondHAL = HAL_GetTick();

	if(millisecond >= 1000){
		millisecond = 0;
		secondNum++;

	}
	if(secondNum >= 60){
		secondNum = 0;
		if(mode != 100){
			minuteNum++;
		}
	}
	if(minuteNum >= 60){
		minuteNum = 0;
		if(mode != 100){
			hourNum++;
		}
	}
	if(hourNum >= 24){
		hourNum = 0;
	}

	//check for editMode
	if(minuteNum < 0){
		minuteNum = 59;
	}
	if(hourNum < 0){
		hourNum = 23;
	}


}


//void displayClock(uint32_t ms){
//
//	char hexString[30];
//	if(halfsecondState == false){ // colon behaviour
//		sprintf(hexString,"%02d:%02d\r", hourNum, minuteNum);
//		HAL_UART_Transmit(&huart3, (uint8_t*) hexString, strlen(hexString), 1000);
//	}else{
//		sprintf(hexString,"%02d %02d\r", hourNum, minuteNum);
//		HAL_UART_Transmit(&huart3, (uint8_t*) hexString, strlen(hexString), 1000);
//	}
//}

void topBarScreen(){


	sprintf(Temp_Buffer_text, "ON");
	ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0.9 + offsetWidth-5, maxHeight*0.1, BLACK, 2, RED);


}

void resetPrevNum(){
	prevSecondNum = -1;
	prevMinuteNum = -1;
	prevHourNum = -1;
}

void hourScreen(bool status, bool isEdit){
	if(prevHourNum != hourNum || isEdit == true){
		if(status == true){
			sprintf(Temp_Buffer_text, "%02d", (int)hourNum);
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0 + offsetWidth-5, maxHeight*0.3, WHITE, 6, BLACK);
		}else{
			sprintf(Temp_Buffer_text, "  ");
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0 + offsetWidth-5, maxHeight*0.3, WHITE, 6, BLACK);
		}
		prevHourNum = hourNum;
	}
}

void colonScreen(bool status){
	if(status == true){
		sprintf(Temp_Buffer_text, ":");
		ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0 + offsetWidth+75, maxHeight*0.35, WHITE, 4, BLACK);
	}else{
		sprintf(Temp_Buffer_text, " ");
		ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0 + offsetWidth+75, maxHeight*0.35, WHITE, 4, BLACK);
	}

}
void minuteScreen(bool status, bool isEdit){
	if(prevMinuteNum != minuteNum || isEdit == true){
		if(status == true){

			sprintf(Temp_Buffer_text, "%02d", (int)minuteNum);
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0 + offsetWidth+100, maxHeight*0.3, WHITE, 6, BLACK);
		}else{
			sprintf(Temp_Buffer_text, "  ");
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0 + offsetWidth+100, maxHeight*0.3, WHITE, 6, BLACK);
		}
		prevMinuteNum = minuteNum;
	}

}
void secondScreen(bool status, bool isEdit){
	if(prevSecondNum != secondNum || isEdit == true){
		if(status == true){
			sprintf(Temp_Buffer_text, "%02d", (int)secondNum);
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0.90 + offsetWidth, maxHeight*0.42, WHITE, 2, BLACK);
		}else{
			sprintf(Temp_Buffer_text, "  ");
			ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0.90 + offsetWidth, maxHeight*0.42, WHITE, 2, BLACK);
		}
		prevSecondNum = secondNum;
	}
}

void displayClockScreen(){

	if(halfsecondState == false){ // colon behaviour
		colonScreen(true);
	}else{
		colonScreen(false);
	}

	secondScreen(true,false);
	minuteScreen(true,false);
	hourScreen(true,false);
}

void editHourScreen(){

	colonScreen(true);
	minuteScreen(true,false);
	secondScreen(true,false);

	if(halfsecondState == false){ // hour
		hourScreen(false,true);
	}else{
		hourScreen(true,true);
	}

}
void editMinuteScreen(){

	colonScreen(true);
	hourScreen(true,false);
	secondScreen(true,false);

	if(halfsecondState == false){ //minute
		minuteScreen(false,true);
	}else{
		minuteScreen(true,true);
	}

}
void editSecondScreen(){

	colonScreen(true);
	hourScreen(true,false);
	minuteScreen(true,false);

	if(halfsecondState == false){  //second
		secondScreen(false,true);
	}else{
		secondScreen(true,true);
	}

}

void bottomBarScreen(){

	uint8_t size = 2;
	//uint16_t maxWidthFull = 320;
	uint8_t bottomHeight = maxHeight*0.87;
	uint8_t bottomWidth1 = maxWidth*0+51;
	uint8_t bottomWidth2 = maxWidth*0.25+51;
	uint8_t bottomWidth3 = maxWidth*0.50+51;
	uint8_t bottomWidth4 = maxWidth*0.75+51;
	uint8_t bottomWidth = 55;

	ILI9341_Draw_Filled_Rectangle_Coord(bottomWidth1, bottomHeight, bottomWidth1+bottomWidth, maxHeight, RED);
	ILI9341_Draw_Filled_Rectangle_Coord(bottomWidth2, bottomHeight, bottomWidth2+bottomWidth, maxHeight, YELLOW);
	ILI9341_Draw_Filled_Rectangle_Coord(bottomWidth3, bottomHeight, bottomWidth3+bottomWidth, maxHeight, CYAN);
	ILI9341_Draw_Filled_Rectangle_Coord(bottomWidth4, bottomHeight, bottomWidth4+bottomWidth, maxHeight, GREEN);

	sprintf(Temp_Buffer_text, "MOD");
	ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0 + offsetWidth, maxHeight*0.9, BLACK, size, RED);

	sprintf(Temp_Buffer_text, "ADJ");
	ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0.25 + offsetWidth, maxHeight*0.9, BLACK, size, YELLOW);

	sprintf(Temp_Buffer_text, "FWD");
	ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0.5 + offsetWidth, maxHeight*0.9, BLACK, size, CYAN);

	sprintf(Temp_Buffer_text, "BWD");
	ILI9341_Draw_Text(Temp_Buffer_text, maxWidth*0.75 + offsetWidth, maxHeight*0.9, BLACK, size, GREEN);

}

void buzzerSound(){
	  htim3.Instance -> CCR1 = (1000-1) * 0.5;
	  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	  HAL_Delay(70);
	  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}




char str[50];
uint8_t cmdBuffer[3];
uint8_t dataBuffer[8];

void assignmentTwo(){

	calculationClock(millisecond);

	if(prevMode != mode || prevModeEdit != modeEdit){
		prevModeEdit = modeEdit;
		resetPrevNum();
	}
	if(prevMode != mode){
		prevMode = mode;
		setHorizontalScreen(BLACK);
		initialState = false;
		bottomBarScreen();
	}


	if(mode == 0){
		topBarScreen();
		displayClockScreen();
	}else if(mode == 100){ // Adjust time

		if(modeEdit == 1){
			editHourScreen();
		}else if(modeEdit == 2){
			editMinuteScreen();
		}else if(modeEdit == 3){
			editSecondScreen();
		}
	}else if(mode == 1){

	}


	//Test huart1 UART PB6 TX - PB15 RX
//	sprintf(Temp_Buffer_text, "AA");
//	HAL_UART_Transmit(&huart1, (uint8_t*) Temp_Buffer_text, strlen(Temp_Buffer_text), 1000);


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
  /* USER CODE BEGIN 2 */
  sprintf(str, "\n\rAM2320 I2C DEMO Starting . . .\n\r");

  HAL_UART_Transmit(&huart3, (uint8_t*) str, strlen(str),200);

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


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */



	  // REAL CODE BEGIN


//	  char stringBuffer[30];
//	  sprintf(stringBuffer, "%d\r\n" , millisecond);
//	  HAL_UART_Transmit(&huart3, (uint8_t*) stringBuffer, strlen(stringBuffer), 200);


	  if(halfsecond == 1){  // interupt every 500 ms
		  halfsecondState = !halfsecondState; // check appearing of colon (:) in clock
		  //displayClock(millisecond);
		  halfsecond = 0;
		  assignmentTwo();
	  }

	  pressButton1 = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7); // pressButton1 is "true" when press, is "false" when not press
	  pressButton2 = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6); // pressButton1 is "true" when press, is "false" when not press
	  pressButton3 = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_5); // pressButton1 is "true" when press, is "false" when not press
	  pressButton4 = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_4); // pressButton1 is "true" when press, is "false" when not press

	  //Buzzer
	  if((pressButton1 == true && isPressButton1 == false) ||
		  (pressButton2 == true && isPressButton2 == false) ||
		  (pressButton3 == true && isPressButton3 == false) ||
		  (pressButton4 == true && isPressButton4 == false)){
		  buzzerSound();
	  }


	  //General Mode
	  if(pressButton1 == true && isPressButton1 == false && mode == 0){ // increase mode only once
		  mode++;
		  isPressButton1 = true;
	  }


	  //Adjust Time Mode
  	  if(pressButton2 == true && isPressButton2 == false && mode == 0){ // initial time when pressButton2
  		  isPressButton2 = true;
  		  prevSecondCounter = millisecondHAL;
  	  }else if(pressButton2 == true && isPressButton2 == true && mode == 0 && millisecondHAL - prevSecondCounter >= 3000){ // hold for 3 seconds
		  buzzerSound();
  		  mode = 100;
		  prevSecondCounter = millisecondHAL;
	  }

  	  //Exit Adjust Time Mode
  	  if(pressButton2 == true && isPressButton2 == false && millisecondHAL - prevSecondCounter >= 1000 && mode == 100){
  		  isPressButton2 = true;
  		  mode = 0;
  		  prevSecondCounter = millisecondHAL;
  	  }

  	  //Edit Mode
	  if(pressButton1 == true && isPressButton1 == false && mode == 100){ // increase mode only once
		  modeEdit++;
		  isPressButton1 = true;
		  if(modeEdit == 4){ // finish loop edit
			  modeEdit = 1; // Reset to hour
			  mode = 0;	// Back to General Mode
		  }
	  }

	  //Forward
	  if(pressButton3 == true && isPressButton3 == false && mode == 100){ // increase value
		  if(modeEdit == 1){
			  hourNum--;
		  }else if(modeEdit == 2){
			  minuteNum--;
		  }else if(modeEdit == 3){
			  secondNum = 0;
		  }
		  halfsecondState = false;
		  resetPrevNum();
		  isPressButton3 = true;
	  }

	  //Backward
	  if(pressButton4 == true && isPressButton4 == false && mode == 100){ // decrease value
		  if(modeEdit == 1){
			  hourNum++;
		  }else if(modeEdit == 2){
			  minuteNum++;
		  }else if(modeEdit == 3){
			  secondNum = 0;
		  }
		  halfsecondState = false;
		  resetPrevNum();
		  isPressButton4 = true;
	  }


  	  //Reset isPressButton
	  if(pressButton1 == false){
		  isPressButton1 = false;
	  }
  	  if(pressButton2 == false){
		  isPressButton2 = false;
	  }
	  if(pressButton3 == false){
		  isPressButton3 = false;
	  }
	  if(pressButton4 == false){
		  isPressButton4 = false;
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
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
uint16_t CRC16_2(uint8_t *ptr, uint8_t length)
{
      uint16_t 	crc = 0xFFFF;
      uint8_t 	s 	= 0x00;

      while(length--) {
        crc ^= *ptr++;
        for(s = 0; s < 8; s++) {
          if((crc & 0x01) != 0) {
            crc >>= 1;
            crc ^= 0xA001;
          } else crc >>= 1;
        }
      }
      return crc;
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
  while(1)
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
