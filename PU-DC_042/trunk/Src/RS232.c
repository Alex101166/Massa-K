
#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "stm32f0xx_it.h"
#include "RS232.h"
#include "Coders.h"
#include <stdbool.h>

uint8_t RS232rxDataBuffer[100];
uint8_t RS232txDataBuffer[100];
uint32_t RS232rxCount;
uint32_t RS232txCount;
bool RS232_Flag_DataReady;

volatile uint8_t rxTimeout;
extern UART_HandleTypeDef huart1;
 
// Инициализация переменных RS232 
void RS232_Init(void)
{

	RS232rxCount=0;
	RS232_Flag_DataReady=false;
//	HAL_UART_Receive_IT(&huart1, (uint8_t *)&dta, 1);
}

//Получить флаг конца приема данных
bool Get_RS232Flag(void)
{
	if (RS232_Flag_DataReady)
	{
		RS232_Flag_DataReady=false;
		return true;
	}
	return false;
}

// Таймоут
void RS232_Timeout(void)
{
	if (rxTimeout==5) 
		RS232rxCount=0;
	if (rxTimeout<7)
		rxTimeout++;
	else rxTimeout=0;
	
}

//void RS232_RxData(

// Прерывание прием символа
void RS232_RxCompete(UART_HandleTypeDef *huart)
{

   uint8_t tmp=huart->Instance->RDR;
  RS232rxDataBuffer[RS232rxCount++]=(uint8_t)tmp;
	rxTimeout=0;
	
	if (IsDataReady(RS232rxDataBuffer,RS232rxCount)>0)
	{
		RS232_Flag_DataReady=true;
	}
}

void RS232_Pool(void)
{

	int DataOutLen;

	
		HAL_UART_Receive_IT(&huart1, (uint8_t *)RS232txDataBuffer, 100);
	
	if (RS232_Flag_DataReady)
	{
							DataOutLen=DecodeData(RS232rxDataBuffer,RS232rxCount,RS232txDataBuffer,&RS232txCount);
						if ((DataOutLen==0)&&(RS232txCount>0)) 
						{
							 HAL_UART_Transmit_IT(&huart1,RS232txDataBuffer,RS232txCount);
						}
				RS232_Flag_DataReady=false;
		
	}
}





