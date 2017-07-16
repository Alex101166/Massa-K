
#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "stm32f0xx_it.h"
#include "ESP-07.h"
#include "Coders.h"
#include "flash.h"
#include <stdbool.h>
#include <stdlib.h> 

#define COMMAND_TEST "+AT\r\n\0"
#define COMMAND_CWMODE "AT+CWMODE=3\r\n\0"
//#define COMMAND_CONNECT "AT+CWJAP=\"DIR-300NRUB6\",\"apokalipsis\"\r\n"
//#define COMMAND_CONNECT "AT+CWJAP=\"MyYNet\",\"yura7777\"\r\n"
#define COMMAND_CONNECT "AT+CWJAP=\"%s\",\"%s\"\r\n\0"
//#define COMMAND_SET_IP "AT+CIPSTA=\"192.168.0.77\",\"192.168.0.1\",\"255.255.255.0\"\r\n"
//AT+CIPSTA="192.168.0.77","192.168.0.1","255.255.255.0"
#define COMMAND_SET_IP "AT+CIPSTA=\"%d.%d.%d.%d\",\"%d.%d.%d.%d\",\"%d.%d.%d.%d\"\r\n\0"
#define COMMAND_SET_MODE "AT+CIPMUX=1\r\n\0"
#define COMMAND_START_SERVER_TCP "AT+CIPSERVER=1,%d\r\n\0"
#define COMMAND_SET_TIMEOUT "AT+CIPSTO=10\r\n\0"
#define COMMAND_GET_LIST_AP "AT+CWLAP\r\n\0"
#define COMMAND_SEND_DATA "AT+CIPSEND=%d,%d\r\n\0"
#define COMMAND_RESET "AT+RST\r\n\0"



#define ESP_RESEIVE_CR 1
#define ESP_RESEIVE_DATA_PRE 2
#define ESP_RESEIVE_DATA_START 3
#define ESP_RESEIVE_DATA_END 4


#define ESP_STATUS_READY 0
#define ESP_STATUS_WAIT_REPLY 1
#define ESP_STATUS_COMPLETE 2
#define ESP_STATUS_WAIT_FOR_DATA 5
#define ESP_STATUS_ERROR 255

#define ESP_RXDATASIZE 100

uint8_t ESP_BUFFER[ 400];

uint8_t* ESPrxDataBuffer;
uint8_t* ESPtxDataBuffer;
uint8_t* ESPtxCommandDataBuffer;

uint32_t ESPrxCount;
uint32_t ESPtxCount;
bool ESP_Flag_DataReady;
bool ESP_DataFlag;
bool ESP_DataOutFlag;
bool ESP_Lap_En;
bool ESP_Lap_Start;
int ESP_DataLenght;
int ESP_DataOutLenght;
int fff;
uint8_t ESP_TIMEOUT;

int IDLink_For_Ansver;

uint8_t ESP_Step;
uint8_t ESP_StepCount;
uint8_t ESP_Status;

volatile uint8_t ESPrxTimeout;
extern UART_HandleTypeDef huart2;
 
// Инициализация переменных RS232 
void ESP_Init(void)
{
	ESPrxDataBuffer = ESP_BUFFER;
	ESPtxDataBuffer =ESP_BUFFER+100;
	ESPrxCount=0;
	ESP_StepCount=ESP_Step=0;
	ESP_Flag_DataReady=false;
	ESP_Status = ESP_STATUS_READY;
	ESP_DataFlag=false;
	ESP_DataOutFlag = false;
	ESP_Lap_En=false;
	ESP_Lap_Start=false;
	HAL_GPIO_WritePin(ESP_RESET_GPIO_Port, ESP_RESET_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin,GPIO_PIN_SET);
	CoderBuffLink(ESP_BUFFER,IFS_WIFI);
	fff=0;
	ESP_TIMEOUT=80;
}

void ESP_Reset(void)
{
	ESP_Step = 100;
}

int ESP_LAP(void)
{
	if (ESP_Status==ESP_STATUS_READY)
	{
		ESP_Step = 27;
		return 1;
	}
	
return 0;	
		
	
}



bool CheckESPData(void)
{
	if (mAllStoredData.WIFI_IP_ADDR==0) return false;
	if (mAllStoredData.WIFI_SSID[0]==0) return false;
	return true;
}

uint8_t ESP_Get_Status(void)
{
	return ESP_Status;
}

void ESP_SendCommand(uint8_t* command,int Len)
{
					HAL_UART_Transmit_IT(&huart2,command,Len);
					ESP_Status = ESP_STATUS_WAIT_REPLY;
	ESPrxCount=0;
	ESP_Flag_DataReady=false;
	ESPrxTimeout=0;
	
}

int ESP_GetAnsver(uint8_t* command,int Len)
{
	return 1;
}
	
uint8_t SendCommandNoAnswer(const char* command,uint8_t NextStepOk,uint8_t NextStepError, uint8_t step)
{
				if (ESP_Status==ESP_STATUS_READY)
				{
					switch (step)
					{
						case 1: 
							sprintf((char*)ESPtxDataBuffer,command,(char*)mAllStoredData.WIFI_SSID,(char*)mAllStoredData.WIFI_PASSWORD);		break;
						case 2:
							sprintf((char*)ESPtxDataBuffer,command,(uint8_t)(mAllStoredData.WIFI_IP_ADDR>>24),(uint8_t)(mAllStoredData.WIFI_IP_ADDR>>16),
																										(uint8_t)(mAllStoredData.WIFI_IP_ADDR>>8),(uint8_t)(mAllStoredData.WIFI_IP_ADDR),
																										(uint8_t)(mAllStoredData.WIFI_IP_MASK>>24),(uint8_t)(mAllStoredData.WIFI_IP_MASK>>16),
																										(uint8_t)(mAllStoredData.WIFI_IP_MASK>>8),(uint8_t)(mAllStoredData.WIFI_IP_MASK),
																										(uint8_t)(mAllStoredData.ETH_IP_GATE>>24),(uint8_t)(mAllStoredData.ETH_IP_GATE>>16),
																										(uint8_t)(mAllStoredData.ETH_IP_GATE>>8),(uint8_t)(mAllStoredData.ETH_IP_GATE));
						break;
						case 3:
							sprintf((char*)ESPtxDataBuffer,command,mAllStoredData.WIFI_IP_PORT);
						break;
						default:
							sprintf((char*)ESPtxDataBuffer,command);
						break;
					}
					ESP_SendCommand(ESPtxDataBuffer,strlen((char*)ESPtxDataBuffer));
					return 1;
				} else
				{
						if (ESP_Status==ESP_STATUS_COMPLETE)
					  {
								ESP_Status=ESP_STATUS_READY;
							  ESP_Step=NextStepOk;
							  ESP_StepCount=0;
						} else  if (ESP_Status!=ESP_STATUS_WAIT_REPLY)
						{
								ESP_Status=ESP_STATUS_READY;
							  ESP_Step=NextStepError;
							  ESP_StepCount=0;
					  }
				}
				return 0;  
}

uint8_t SendCommandDataStart(  uint8_t NextStepOk,uint8_t NextStepError)
{
				if (ESP_Status==ESP_STATUS_READY)
				{
					ESPtxCommandDataBuffer = ESP_BUFFER+50;
					sprintf((char*)ESPtxCommandDataBuffer,COMMAND_SEND_DATA,IDLink_For_Ansver,ESP_DataOutLenght);
					ESP_SendCommand(ESPtxCommandDataBuffer,strlen(COMMAND_SEND_DATA));
				} else
				{
						if (ESP_Status==ESP_STATUS_WAIT_FOR_DATA)
					  {
								ESP_Status=ESP_STATUS_READY;
							  ESP_Step=NextStepOk;
							  ESP_StepCount=0;
						} else  if (ESP_Status!=ESP_STATUS_WAIT_REPLY)
						{
								ESP_Status=ESP_STATUS_READY;
							  ESP_Step=NextStepError;
							  ESP_StepCount=0;
					  }
				}
				return 0;
}

uint8_t SendCommandData(int Len,  uint8_t NextStepOk,uint8_t NextStepError)
{
				if (ESP_Status==ESP_STATUS_READY)
				{
//					sprintf((char*)ESPtxDataBuffer,COMMAND_SEND_DATA);
					ESP_SendCommand(ESPtxDataBuffer,Len);
						ESP_Flag_DataReady=false;
							ESP_DataFlag=false;
						ESP_DataOutFlag = false;

				} else
				{
						if (ESP_Status==ESP_STATUS_COMPLETE)
					  {
								ESP_Status=ESP_STATUS_READY;
							  ESP_Step=NextStepOk;
							  ESP_StepCount=0;
						} else  if (ESP_Status!=ESP_STATUS_WAIT_REPLY)
						{
								ESP_Status=ESP_STATUS_READY;
							  ESP_Step=NextStepError;
							  ESP_StepCount=0;
					  }
				}
				return 0;
}


void SendData(void)
{
						sprintf((char*)ESPtxDataBuffer,COMMAND_SEND_DATA,0,13);
					ESP_SendCommand(ESPtxDataBuffer,strlen(COMMAND_SEND_DATA));

}

//Получить флаг конца приема данных
bool Get_ESPFlag(void)
{
	if (ESP_Flag_DataReady)
	{
		ESP_Flag_DataReady=false;
		return true;
	}
	return false;
}



// Таймоут
int fff;
char* ttt;
void ESP_Timeout(void)
{
	//if (ESPrxTimeout==5) 
	//	ESPrxCount=0;
	
		ESP_StepCount++;
	// проверка таймаута
	if (ESPrxTimeout<ESP_TIMEOUT)
		ESPrxTimeout++;
	else 
	{
		if (ESP_Status == ESP_STATUS_WAIT_REPLY)
			ESP_Status = ESP_STATUS_ERROR;
		ESPrxTimeout=0;
		ESP_DataOutFlag=false;
	}
}

//void RS232_RxData(
bool StringCompare(char * str1, const char* str2)
{
	int i;
	if (strlen(str1)<strlen(str2)) return false;
	for (int i=0;i<strlen(str2);i++)
	{
		if (str1[i]!=str2[i]) return false;
	}
	return true;
}


int IsESPDataReady(char* DataBuffer,uint32_t rxCount, uint8_t Channal)
{
	int i,k;
	uint32_t txLen;
		// Проверили на ОК
			if (Channal==ESP_RESEIVE_CR)
			{
				ttt= strrchr((char*)ESPrxDataBuffer,0x0a);
					if ((ttt!=0)&&(((uint32_t)ttt-(uint32_t)ESPrxDataBuffer)>4))
					{
						
						//if ((*(ttt-3) == 'O')&&(*(ttt-2) == 'K')&&(*(ttt-1) == 0x0d))
						if (StringCompare(ttt-3,"OK\r\n"))
						{
							ESPrxCount=0;
							return 1;
						}
					}
		// Проверили на ERROR
					if ((ttt!=0)&&(((uint32_t)ttt-(uint32_t)ESPrxDataBuffer)>7))					
					{
					//		if ((*(ttt-6) == 'E')&&(*(ttt-5) == 'R')&&(*(ttt-4) == 'R')&&(*(ttt-3) == 'O')&&(*(ttt-2) == 'R')&&(*(ttt-1) == 0x0d))
						if (StringCompare(ttt-6,"ERROR\r\n"))
						{
							ESPrxCount=0;
							return 2;
						}
					if (StringCompare(ttt-5,"FAIL\r\n"))
						{
							ESPrxCount=0;
							return 3;
						}
					}
			
					//Проверили на LAP
					if (ESP_Lap_En)
					{
						if (ESP_Lap_Start) 
						{
							ESPtxDataBuffer[0]=0;
							ESP_Lap_Start=false;
						}
						
						if (ttt>0)
						{
							k=0;i=strlen((char*)ESPtxDataBuffer);
							
							ttt = strchr((char*)ESPrxDataBuffer,'"');
							if (ttt>0)
							{
								ttt++;
									while ((ttt[k]!=0x0a)&&(ttt[k]!='"'))
									{
										ESPtxDataBuffer[i+k]=ttt[k];
										k++;
									}
										ESPtxDataBuffer[i+k]=0x0d;k++;
									ESPtxDataBuffer[i+k]=0x0A;k++;
									ESPtxDataBuffer[i+k]=ESPtxDataBuffer[i+k+1]=ESPtxDataBuffer[i+k+2]=ESPtxDataBuffer[i+k+3]=0;
									ESPrxCount=0;
							}
							
							/*				while (ESPrxDataBuffer[k]!=0x0a)
									{
										ESPtxDataBuffer[i+k]=ESPrxDataBuffer[k];
										k++;
									}
										ESPtxDataBuffer[i+k]=0x0d;k++;
									ESPtxDataBuffer[i+k]=0x0A;k++;
									ESPtxDataBuffer[i+k]=0;
									ESPrxCount=0;
									*/
	
						}
					}
					
				}
	 // Проверка на данные		
			if (Channal==ESP_RESEIVE_DATA_PRE)
			{
   					if (StringCompare((char*)ESPrxDataBuffer,"+IPD"))
						{
							ESP_DataFlag=true;
							ESP_DataLenght=0;
						}
						//else 
						//	ESP_DataFlag=false;
			}
			
			if (Channal==ESP_RESEIVE_DATA_START)
			{
					{
						ESPrxDataBuffer[ESPrxCount-1]=0;
						ttt= strrchr((char*)ESPrxDataBuffer,',');
						ESP_DataLenght = atoi((char*)(ttt+1));
						*ttt=0;
						ttt= strrchr((char*)ESPrxDataBuffer,',');
						IDLink_For_Ansver = atoi((char*)(ttt+1));


//						ESP_DataLenght=8;
						ESPrxCount=0;
					}
			}
			
			if (Channal==ESP_RESEIVE_DATA_END)
			{
		//		sprintf((char*)ESPtxDataBuffer,COMMAND_SEND_DATA,);
				DecodeData(ESPrxDataBuffer,ESP_DataLenght,ESPtxDataBuffer,&txLen);
				ESP_DataOutLenght=txLen;
				ESP_Step=78;
				ESPrxCount=0;
				//ESP_DataFlag=false;
				ESP_DataLenght=0;
				ESP_DataOutLenght=txLen;
				ESP_DataOutFlag=true;
			}
				
			return 0;		
					
}
	

#define ESP_RESEIVE_CR 1
#define ESP_RESEIVE_DATA_PRE 2
#define ESP_RESEIVE_DATA_START 3
#define ESP_RESEIVE_DATA_END 4

// Прерывание прием символа
int Res;
void ESP_RxCompete(UART_HandleTypeDef *huart)
{
		
	
   uint8_t tmp=huart->Instance->RDR;
		if ((tmp=='+')&&(!ESP_DataOutFlag))
		{
//			if (!ESP_Lap_En)
				ESPrxCount=0;
//				StoreLapData();
		
		}
			
	if (ESPrxCount<100)
	{
		ESPrxDataBuffer[ESPrxCount++]=(uint8_t)tmp;
		ESPrxDataBuffer[ESPrxCount] =0;
		ESPrxTimeout=0;

		if ((tmp==0x0a)&&(!ESP_DataFlag))
		{
			
			Res = IsESPDataReady((char*)ESPrxDataBuffer,ESPrxCount,ESP_RESEIVE_CR);
			if (Res>0)
			{
				ESP_Flag_DataReady=true;
			}
	  }
		if ((ESPrxCount==4)&&(!ESP_DataFlag))
		{
					Res = IsESPDataReady((char*)ESPrxDataBuffer,ESPrxCount,ESP_RESEIVE_DATA_PRE);
		}
		if ((tmp==':')&&(ESP_DataFlag)&&(!ESP_DataOutFlag))
		{
			Res = IsESPDataReady((char*)ESPrxDataBuffer,ESPrxCount,ESP_RESEIVE_DATA_START);
		}
		if ((ESP_DataFlag)&&(ESPrxCount == ESP_DataLenght)&&(!ESP_DataOutFlag))
		{
				Res = IsESPDataReady((char*)ESPrxDataBuffer,ESPrxCount,ESP_RESEIVE_DATA_END);
		}
		if (ESP_DataOutFlag)
		{
			if (tmp=='>')
			{
				ESP_DataFlag = false;
				ESP_Status=ESP_STATUS_WAIT_FOR_DATA;
			}
		}
	} 
	
}

void ESP_Pool(void)
{

//	int DataOutLen;

	
		HAL_UART_Receive_IT(&huart2, (uint8_t *)ESPtxDataBuffer, 100);
	
	if (ESP_Flag_DataReady)
	{
		ESP_GetAnsver(ESPrxDataBuffer,ESPrxCount);
		ESP_Flag_DataReady=false;
		ESP_Status = ESP_STATUS_COMPLETE;
		
	}
	
	switch (ESP_Step)
	{
		case 0: // Проверка подключения
		
			if (ESP_StepCount>50)
			{
				if (CheckESPData())
				SendCommandNoAnswer(COMMAND_TEST,1,0,0);
				else
					ESP_StepCount=0;
				
		  } 
			break;
		case 1: //Установка режима сервер-клиент
				if (ESP_StepCount>10)
			{
								SendCommandNoAnswer(COMMAND_CWMODE,2,0,0);
			  } 
			break;
		case 2: // соединение
				if (ESP_StepCount>10)
			{
					SendCommandNoAnswer(COMMAND_CONNECT,3,0,1);
		  } 
			break;
		case 3: // Set IP
				if (ESP_StepCount>10)
			{
				SendCommandNoAnswer(COMMAND_SET_IP,4,0,2);
		  } 
			break;
		case 4: // Set IP
				if (ESP_StepCount>10)
			{
				SendCommandNoAnswer(COMMAND_SET_MODE,5,0,0);

		  } 
			break;
		case 5: // Set IP
				if (ESP_StepCount>40)
			{
				SendCommandNoAnswer(COMMAND_START_SERVER_TCP,76,0,3);

		  } 
			break;
		case 76:
			ESP_TIMEOUT=8;
			ESP_Step=77;
		break;
		
		case 78:
			SendCommandDataStart(79,77);
		break;
		case 79:
			SendCommandData(ESP_DataOutLenght,77,77);
			break;
		case 100:
			SendCommandNoAnswer(COMMAND_RESET,0,0,0);
			ESP_Status=ESP_STATUS_READY;
			ESP_Step=0;
			ESP_StepCount=0;
			ESP_TIMEOUT=80;
		break;

		case 27: // Set IP
//				if (ESP_StepCount>10)
				ESP_TIMEOUT=80;
			{
				if (SendCommandNoAnswer(COMMAND_GET_LIST_AP,28,0,0)>0)
				{
						ESP_Lap_En=true;
						ESP_Lap_Start=true;
				}

		  } 
			break;
		case 28:
			  FlashStoreLAP((char*) ESPtxDataBuffer);
				ESP_Step=76;
		break;
		default:
			
		break;
	}
 
	

/*	
	if (ESP_Flag_DataReady)
	{
							DataOutLen=DecodeData(ESPrxDataBuffer,ESPrxCount,ESPtxDataBuffer,&ESPtxCount);
						if ((DataOutLen==0)&&(ESPtxCount>0)) 
						{
							 HAL_UART_Transmit_IT(&huart2,ESPtxDataBuffer,ESPtxCount);
						}
				ESP_Flag_DataReady=false;
		
	}
	*/
}

unsigned char Is_ESP_Work(void)
{
	if (ESP_Step>5) return 1;
	else
	return 0;
}
		
	



