/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */

/******************************************************************************/
/*                             ЗАГОЛОВОЧНЫЕ ФАЙЛЫ                             */
/******************************************************************************/

#include "I2C.h"
#include "USB.h"
#include "types_defines.h"
#include "ifs_ethernet.h"
#include "coders.h"
#include "rs232.h"
#include "flash.h"
#include "esp-07.h"

/******************************************************************************/
/*                       ЛОКАЛЬНЫЕ МАКРООПРЕДЕЛЕНИЯ                           */
/******************************************************************************/

// Макс. кол-во подключений по TCP для интерфейса Ethernet
#define MAX_TCP_CONNECT_CNT 5

// Макс. длина пакета данных (в байтах), посылаемых по TCP для интерфейса Ethernet
#define MAX_ETH_PACKET_LEN  200

/******************************************************************************/
/*                            ЛОКАЛЬНЫЕ ТИПЫ ДАННЫХ                           */
/******************************************************************************/

/******************************************************************************/
/*                               ГЛОБАЛЬНЫЕ ДАННЫЕ                            */
/******************************************************************************/

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

// Структура вывода веса на индикатор
Weight_Indicator_t  mW_Indicator;

// Структура вывода веса
Weight_t  mWeight;

// Структура блока информации датчика DLC
Info_DLC_t  mInfo_DLC;

// Структура блока информации блока управления
Info_CU_t  mInfo_CU;

// Структура блока информации блока индикации
Info_IU_t  mInfo_IU;

// Структура блока сохраненных данных
AllStoredData_t mAllStoredData;

//
const uint32_t BLOCK_VERSIOIN = 0x050701;
const uint64_t BLOCK_SERIAL_NUM  __attribute__((at(0x8000800))) = 0x000000110617ffa0;
const uint32_t BLOCK_DATE  __attribute__((at(0x8000808))) = 0x00170917;
const uint8_t BLOCK_MAC_5  __attribute__((at(0x8000810))) = 0x00;
const uint8_t BLOCK_MAC_4  __attribute__((at(0x8000811))) = 0x1e;
const uint8_t BLOCK_MAC_3  __attribute__((at(0x8000812))) = 0xfc;
const uint8_t BLOCK_MAC_2  __attribute__((at(0x8000813))) = 0x20;
const uint8_t BLOCK_MAC_1  __attribute__((at(0x8000814))) = 0x00;
const uint8_t BLOCK_MAC_0  __attribute__((at(0x8000815))) = 0x01;
//const uint8_t BLOCK_MAC[6]  __attribute__((at(0x8000810))) = {0x00, 0x1E, 0xFC,0x20, 0x00, 0x01};
const char 		 BLOCK_NAME[]  = "Блок интерфейсный МК\0";
const uint32_t STORE_DATA_AREA[128]  __attribute__((at(0x8001000)));
const char 		 STORE_LAP[512]  __attribute__((at(0x8001200)));
// 
const char 		 MEMORY_RESERV[1024]  __attribute__((at(0x8001400)));
/******************************************************************************/
/*                              ЛОКАЛЬНЫЕ ДАННЫЕ                              */
/******************************************************************************/

/******************************************************************************/
/*                         ПРОТОТИПЫ ЛОКАЛЬНЫХ ФУНКЦИЙ                        */
/******************************************************************************/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM14_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

///////////////////// Для отладки модуля Ethernet - END /////////////////////
                                 
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/******************************************************************************/
/*                             ГЛОБАЛЬНЫЕ ФУНКЦИИ                             */
/******************************************************************************/

//
extern bool Get_TimerFlag(void);

///////////////////////////////////////////////////////////////////////////////////////////////////
//                              Инициализация интерфейса Ethernet                                //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
void ethernet_interface_init(void)
{
    uint32_t i;
    // Сетевая конфигурация
    static wiz_NetInfo wiz_net_inf = { .mac  = { BLOCK_MAC_5, BLOCK_MAC_4, BLOCK_MAC_3, 
                                                 BLOCK_MAC_2, BLOCK_MAC_1, BLOCK_MAC_0 },
                                       .dns  = {0,0,0,0},
                                       .dhcp = NETINFO_STATIC };
    //
    // Заполнение структуры настроек
    for (i=0; i<4; i++)                                
    {
        *(wiz_net_inf.ip + i) = *((int8_t*)&mAllStoredData.ETH_IP_ADDR + (3 - i));
        *(wiz_net_inf.sn + i) = *((int8_t*)&mAllStoredData.ETH_IP_MASK + (3 - i));
        *(wiz_net_inf.gw + i) = *((int8_t*)&mAllStoredData.ETH_IP_GATE + (3 - i));
    }
                               
    // Инициализация модуля Ethernet (если IP==0, то отключение интерфейса)
    ifs_ethernet_init((mAllStoredData.ETH_IP_ADDR == 0) ? NULL : &wiz_net_inf);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                           Периодический опрос интерфейса Ethernet                             //
///////////////////////////////////////////////////////////////////////////////////////////////////
//
void ethernet_interface_poll(void)
{
    static bool ifs_inited = false;
    
    // Соединение UDP
    ether_connection_t con1 = { .conn_type = CONN_UDP,
                                .reciv_packet_len = MAX_ETH_PACKET_LEN,  // Чтение всех пришедших данных
                                .reciv_timeout_ms = 0 };                 // без ожидания
    
    // Соединение TCP-сервер
    ether_connection_t con2 = { .conn_type = CONN_TCP_SERVER,
                                .reciv_packet_len = MAX_ETH_PACKET_LEN,  // Чтение всех пришедших данных
                                .reciv_timeout_ms = 0 };                 // без ожидания
    int32_t i;
    //
    switch (ifs_ethernet_interface_status_get())
    {
        // В этом сост. интерфейс готов к установлению соединений
        case IFS_STATE_INITED:
            //
            if (!ifs_inited)
            {
                ifs_inited = true;
                //
                con1.local_port = con2.local_port = mAllStoredData.ETH_IP_PORT;
                // Создание UDP соединения
                con1.reciv_transmit_callback = DecodeData;
                ifs_ethernet_connection_open(&con1);
                //
                // Создание TCP серверов
                con2.reciv_transmit_callback = DecodeData;
                for (i=0; i<MAX_TCP_CONNECT_CNT; i++)
                {
                    ifs_ethernet_connection_open(&con2);
                }
                // Привязка входного буфера к интерфейсу (один буфер на все соединения)
                CoderBuffLink(ifs_ethernet_rxbuf_ref_get(), IFS_ETHERNET);
            }
            break;
        
        default:
            //                          
            ifs_inited = false;
            break;
    }                                
    //
    ifs_ethernet_tick();
}

//
int ReadStoredData(void)
{
    FlashReadData((uint32_t*)( &mAllStoredData));
	return 0;
}

//	
int StoreNewData(void)
{
	FlashStoreData((uint32_t*)( &mAllStoredData));
	return 0;
}
        
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_ADC_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_TIM14_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN 2 */
	// Stert timer 14
  HAL_TIM_Base_Start(&htim14);
  HAL_TIM_Base_Start_IT(&htim14);
		
  // Включили питание индикатора
//          HAL_Delay(200);
  HAL_GPIO_WritePin(POWER_EN_GPIO_Port, POWER_EN_Pin,GPIO_PIN_RESET);
		
  ReadStoredData();

    //!!!!!!!!!!!!!!!! Для отладки (в релизной версии удалить) !!!!!!!!!!!!!!!!!!!!!!
                                
    mAllStoredData.ETH_IP_ADDR = 0xC0A801F1;   // 192.168.1.241
    mAllStoredData.ETH_IP_MASK = 0xFFFFFF00;   // 255.255.255.0
    mAllStoredData.ETH_IP_GATE = 0xC0A80106;   // 192.168.1.6
    mAllStoredData.ETH_IP_PORT = 5000;
    
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  //
  CodersInit();

  //
  I2C_Init();
  
  //
  USB_Init();
  
  //
  RS232_Init();
	
  //
  ESP_Init();

  //
  ethernet_interface_init();
    
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (Get_TimerFlag()) // 100ms
    {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
        RS232_Timeout();
        I2C_Timer();
        ESP_Timeout();
        ethernet_interface_poll();
    }
      
    //
    I2C_Pool();
    
    //
    USB_Pool();
    
    //
    RS232_Pool();
		
    //
    ESP_Pool();
        
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  
}

/******************************************************************************/
/*                              ЛОКАЛЬНЫЕ ФУНКЦИИ                             */
/******************************************************************************/
	
  /* USER CODE END 3 */
/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC init function */
static void MX_ADC_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel to be converted. 
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel to be converted. 
    */
  sConfig.Channel = ADC_CHANNEL_6;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 80;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_ENABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* SPI2 init function */
static void MX_SPI2_Init(void)
{

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM14 init function */
static void MX_TIM14_Init(void)
{

  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 24000;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 100;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
   huart1.Init.BaudRate = 57600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
   huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
//  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	 huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_SWAP_INIT;
  huart2.AdvancedInit.Swap = UART_ADVFEATURE_SWAP_ENABLE;

  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, ESP_SLEEP_Pin|TEST_POINT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ESP_RESET_Pin|ESP_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, VBAT_Pin|POWER_EN_Pin|LED_Pin, GPIO_PIN_RESET);
	
	/*Configure GPIO pin : RS232_TARE_Pin */
GPIO_InitStruct.Pin = RS232_TARE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RS232_TARE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ETH_RST_Pin|ETH_SCSn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : ESP_SLEEP_Pin TEST_POINT_Pin */
  GPIO_InitStruct.Pin = ESP_SLEEP_Pin|TEST_POINT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : ESP_RESET_Pin ESP_EN_Pin */
  GPIO_InitStruct.Pin = ESP_RESET_Pin|ESP_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : CHARGE_EN_Pin */
  GPIO_InitStruct.Pin = CHARGE_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CHARGE_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ETH_INT_Pin */
  GPIO_InitStruct.Pin = ETH_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ETH_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : VBAT_Pin POWER_EN_Pin LED_Pin */
  GPIO_InitStruct.Pin = VBAT_Pin|POWER_EN_Pin|LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ETH_RST_Pin ETH_SCSn_Pin */
  GPIO_InitStruct.Pin = ETH_RST_Pin|ETH_SCSn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RS232_S1_Pin */
  GPIO_InitStruct.Pin = RS232_S1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RS232_S1_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
