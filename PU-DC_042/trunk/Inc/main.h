/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/******************************************************************************/
/*                             ЗАГОЛОВОЧНЫЕ ФАЙЛЫ                             */
/******************************************************************************/

#include "types_defines.h"

/******************************************************************************/
/*                         ГЛОБАЛЬНЫЕ МАКРООПРЕДЕЛЕНИЯ                        */
/******************************************************************************/

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define ESP_SLEEP_Pin GPIO_PIN_13
#define ESP_SLEEP_GPIO_Port GPIOC
#define TEST_POINT_Pin GPIO_PIN_14
#define TEST_POINT_GPIO_Port GPIOC
#define ESP_RESET_Pin GPIO_PIN_0
#define ESP_RESET_GPIO_Port GPIOA
#define ESP_EN_Pin GPIO_PIN_1
#define ESP_EN_GPIO_Port GPIOA
#define ESP_TX_Pin GPIO_PIN_2
#define ESP_TX_GPIO_Port GPIOA
#define ESP_RX_Pin GPIO_PIN_3
#define ESP_RX_GPIO_Port GPIOA
#define V_IN_Pin GPIO_PIN_4
#define V_IN_GPIO_Port GPIOA
#define CHARGE_EN_Pin GPIO_PIN_5
#define CHARGE_EN_GPIO_Port GPIOA
#define V_AK_Pin GPIO_PIN_6
#define V_AK_GPIO_Port GPIOA
#define ETH_INT_Pin GPIO_PIN_0
#define ETH_INT_GPIO_Port GPIOB
#define ETH_INT_EXTI_IRQn EXTI0_1_IRQn
#define VBAT_Pin GPIO_PIN_1
#define VBAT_GPIO_Port GPIOB
#define ETH_RST_Pin GPIO_PIN_2
#define ETH_RST_GPIO_Port GPIOB
#define ETH_CLK_Pin GPIO_PIN_10
#define ETH_CLK_GPIO_Port GPIOB
#define ETH_SCSn_Pin GPIO_PIN_12
#define ETH_SCSn_GPIO_Port GPIOB
#define ETH_MISO_Pin GPIO_PIN_14
#define ETH_MISO_GPIO_Port GPIOB
#define ETH_MOSI_Pin GPIO_PIN_15
#define ETH_MOSI_GPIO_Port GPIOB
#define RS232_TX_Pin GPIO_PIN_9
#define RS232_TX_GPIO_Port GPIOA
#define RS232_RX_Pin GPIO_PIN_10
#define RS232_RX_GPIO_Port GPIOA
#define RS232_TARE_Pin GPIO_PIN_3
#define RS232_TARE_GPIO_Port GPIOB
#define RS232_S1_Pin GPIO_PIN_4
#define RS232_S1_GPIO_Port GPIOB
#define POWER_EN_Pin GPIO_PIN_5
#define POWER_EN_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_8
#define LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/******************************************************************************/
/*                            ГЛОБАЛЬНЫЕ ТИПЫ ДАННЫХ                          */
/******************************************************************************/

// Структура вывода веса на индикатор
// Команда блока управления 6.1
typedef struct 
{
	uint8_t data[7];
	bool Change;
} Weight_Indicator_t;

// Структура вывода веса
// Команда блока управления 7
typedef struct
{
	uint8_t Let_Zero;
	uint8_t Led_NET;
	uint8_t	Resolution;
	int32_t Data;
	bool Change;
	uint8_t Stable;	
} Weight_t;

// Структура блока информации датчика DLC
// Команды 1.0, 1.1 адрес 0x10
typedef struct 
{
	int32_t Dev_ID;
	int64_t Dev_SN;
	uint32_t ControlCode;
	uint8_t Latitude;
} Info_DLC_t;

// Структура блока информации блока управления
// Команды 1.0, 1.1 адрес 0x20 или 0x24
typedef struct 
{
	int32_t Dev_ID;
	int64_t Dev_SN;
} Info_CU_t;

// Структура блока информации блока индикации
// Команды 1.0, 1.1 адрес 0x30 или 0x32
typedef struct 
{
	int32_t Dev_ID;
	int64_t Dev_SN;
} Info_IU_t;

// Структура, содержащая сохраненные данные
// Заполняется при старте
typedef struct 
{
    // Общие
    uint8_t  ScaleName[25];
    uint8_t  ScaleDescription[25];
    // TCP-IP
    uint32_t ETH_IP_ADDR;
    uint32_t ETH_IP_MASK;
    uint32_t ETH_IP_GATE;
    uint16_t ETH_IP_PORT;
    // WiFi-IP
    uint32_t WIFI_IP_ADDR;
    uint32_t WIFI_IP_MASK;
    uint32_t WIFI_IP_GATE;
    uint16_t WIFI_IP_PORT;
    uint8_t  WIFI_SSID[32];
    uint8_t  WIFI_PASSWORD[64];
    // Reserv not use
    uint32_t NOT_USE[4];	
} AllStoredData_t;

/******************************************************************************/
/*                              ГЛОБАЛЬНЫЕ ДАННЫЕ                             */
/******************************************************************************/

// Структура вывода веса на индикатор
extern Weight_Indicator_t  mW_Indicator;

// Структура вывода веса
extern Weight_t  mWeight;

// Структура блока информации датчика DLC
extern Info_DLC_t  mInfo_DLC;

// Структура блока информации блока управления
extern Info_CU_t  mInfo_CU;

// Структура блока информации блока индикации
extern Info_IU_t mInfo_IU;

// Структура блока сохраненных данных
extern AllStoredData_t mAllStoredData;

//
extern const uint32_t BLOCK_VERSIOIN;
extern const uint64_t BLOCK_SERIAL_NUM;
extern const uint32_t STORE_DATA_AREA[128];
extern const uint32_t BLOCK_DATE;
extern const char 		BLOCK_NAME[];
extern const char 	  STORE_LAP[512];


/******************************************************************************/
/*                         ПРОТОТИПЫ ГЛОБАЛЬНЫХ ФУНКЦИЙ                       */
/******************************************************************************/

extern int StoreNewData(void);
extern int ReadStoredData(void);
extern void ethernet_interface_init(void);

/* USER CODE END Private defines */

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
