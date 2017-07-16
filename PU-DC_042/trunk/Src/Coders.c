
#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "RS232.h"
#include "ESP-07.h"
#include "coders.h"
#include "ifs_ethernet.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

uint32_t pInterface[4];
uint8_t CurrentInterface;

int CodeData( unsigned char Command,unsigned char *CodeData, int Lenght);
int GenerateAnsver( unsigned char Command,unsigned char *TxCodeData, unsigned char *RxDecodeData, int Lenght);

	
void CodersInit(void)
{
		pInterface[0]=pInterface[1]=pInterface[2]=pInterface[3]=0;
	
	
}

//Привязка входного буфера к интерфейсу
int rrr ;
void CoderBuffLink(unsigned char* buffer, interface_type_t Interface)
{
	if (Interface>3) return;
	pInterface[Interface]=(uint32_t)buffer;
	
}

uint16_t CRC16(uint8_t *buf, int len)
{
//	word CRC16(word crc, byte *buf, word len)
//{
	uint16_t bits, k;
	uint16_t accumulator, temp;
	
  uint16_t crc=0;
	for( k = 0; k<len; k++ )
	{
		accumulator = 0;
		temp = (crc>>8)<<8;
		for( bits = 0; bits < 8; bits++ )
		{
			if( (temp ^ accumulator) & 0x8000 )
				accumulator = (accumulator << 1) ^ 0x1021;
			else
				accumulator <<= 1;
			temp <<= 1;
		}                                                 
		crc = accumulator^(crc<<8)^(buf[k]&0xff);
	}         
	return crc;
}                    

/*  
     Процедура расшифровки данных
			Возвращает длину посылки
			0 -- команда принята не до конца
			-1 -- ошибка CRC
			-2 -- ошибка старта
*/

void SetDataFromU32(uint8_t* buff,uint32_t Data)
{
						*(buff) = (uint8_t)Data;buff++;
					*(buff) = (uint8_t)(Data>>8);buff++;
					*(buff) = (uint8_t)(Data>>16);buff++;
					*(buff) = (uint8_t)(Data>>24);
}	

uint32_t SetDataToU32(uint8_t* buff)
{
	int i;
	uint32_t Data;
	
					Data=0;
				for (i=0;i<4;i++)
		    {
					Data += (uint32_t)(*(buff+i))<<(i*8);
				}
				return Data;
}	


int IsDataReady(unsigned char *RxDecodeData, unsigned int FullLenght)
{
		int Lenght;
uint16_t crc_tt;
	// Если не совпадает протокол
	if (FullLenght !=0)
	{
			if (RxDecodeData[0]!=0xf8)
				return -3;
	}
	//Если сообщение слишком короткое
	if (FullLenght<6) 
		return -4;
	// Если не совпадают стартовые байты
	if ((RxDecodeData[0]!=0xf8)||(RxDecodeData[1]!= 0x55)||(RxDecodeData[2]!= 0xce))
		return -2;
	// Посчитали длину тела сообщения
		Lenght = RxDecodeData[3]+(int)(RxDecodeData[4]<<8);
	// Если сообщение принято не до конца
	if (FullLenght< (Lenght+7) ) 
		return -4;
	// Посчитали КС
	crc_tt= CRC16(& RxDecodeData[5],Lenght);

	// Сравнини контрольные суммы
	if (((RxDecodeData[FullLenght-1])!=(uint8_t)(crc_tt>>8))||((RxDecodeData[FullLenght-2])!=(uint8_t)(crc_tt)))
		return -1;
	
	return Lenght;
}


	
int DecodeData(unsigned char *RxDecodeData, unsigned int RxDataLen, unsigned char *TxCodeData, unsigned int* TxDataLen)
{
//uint16_t crc;
	int Len;
	uint8_t Command;
	int i;
	
		* TxDataLen = 0;
	//Проверили корректность данных
	Len = IsDataReady(RxDecodeData,RxDataLen);
	
	CurrentInterface=0;
	for (i=0;i<4;i++)
	{
			if (pInterface[i]==(uint32_t)RxDecodeData ) CurrentInterface=i;
	}
	
	// Если ошибка КС
	if (Len == -1)
	{
			Len=0;
			Command = CMD_TCP_NACK;
			//Подготовили данные к отправке
			 CodeData(Command,TxCodeData,Len);
			*TxDataLen = Len;
			return -1;
	} else
	if (Len>=0)
	{

		
			Command = RxDecodeData[5];
			
			for (i=0;i<(Len-1);i++)
			{
				RxDecodeData[i]=RxDecodeData[i+6];
			}
			
			Len= GenerateAnsver(Command,TxCodeData,RxDecodeData,Len-1);
			*TxDataLen = Len;
			
			return 0;
		} else
		return Len;

}

unsigned char LastCommand;
int lll;
int GenerateAnsver( unsigned char Command,unsigned char *TxCodeData, unsigned char *RxDecodeData, int Lenght)
{
	int Len;
	int32_t t_I32;
	int64_t t_I64;	
	int i,k;
	int tmp;
	
	unsigned char CommandOut;
	unsigned char* pChar;

	pChar = TxCodeData;
	LastCommand=Command;
	switch (Command)
	{
		// Команда общего опроса
		case CMD_UDP_POLL:
		//Fixed
			*(TxCodeData) = 0x00;TxCodeData++;
			*(TxCodeData) = 0x02;TxCodeData++;			
		//IDN
		  *(TxCodeData) = (uint8_t)(BLOCK_VERSIOIN>>16);TxCodeData++;
		  *(TxCodeData) = (uint8_t)(BLOCK_VERSIOIN>>8);TxCodeData++;
		  *(TxCodeData) = (uint8_t)(BLOCK_VERSIOIN);TxCodeData++;
		//SerialNum
			*(TxCodeData) = (uint8_t)(BLOCK_SERIAL_NUM);TxCodeData++;
		  *(TxCodeData) = (uint8_t)(BLOCK_SERIAL_NUM>>8);TxCodeData++;
		  *(TxCodeData) = (uint8_t)(BLOCK_SERIAL_NUM>>16);TxCodeData++;
		  *(TxCodeData) = (uint8_t)(BLOCK_SERIAL_NUM>>24);TxCodeData++;
		  *(TxCodeData) = (uint8_t)(BLOCK_SERIAL_NUM>>32);TxCodeData++;

		//Fixed
	//		*(TxCodeData) = 0;TxCodeData++;
			*(TxCodeData) = 1;TxCodeData++;	
		//Serv
			*(TxCodeData) = 0x77;TxCodeData++;
		//ControlCode
		/*	*(TxCodeData) = (uint8_t)(mInfo_DLC.ControlCode>>24);TxCodeData++;
		  *(TxCodeData) = (uint8_t)(mInfo_DLC.ControlCode>>16);TxCodeData++;
		  *(TxCodeData) = (uint8_t)(mInfo_DLC.ControlCode>>8);TxCodeData++;
		  *(TxCodeData) = (uint8_t)(mInfo_DLC.ControlCode);TxCodeData++;*/
				SetDataFromU32(TxCodeData,mInfo_DLC.ControlCode);TxCodeData+=4;
			
			for (i=0;i<10;i++)
			{
					*(TxCodeData) = 0;TxCodeData++;
			}
		
			Len = 26;
			CommandOut=CMD_UDP_RES_ID;
			break;
			// Команд опроса веса
		case CMD_TCP_GET_WEIGHT:
			if (mWeight.Change)
			{
		/*			*(TxCodeData) = (uint8_t)mWeight.Data;TxCodeData++;
					*(TxCodeData) = (uint8_t)(mWeight.Data>>8);TxCodeData++;
					*(TxCodeData) = (uint8_t)(mWeight.Data>>16);TxCodeData++;
					*(TxCodeData) = (uint8_t)(mWeight.Data>>24);TxCodeData++;*/
					SetDataFromU32(TxCodeData,mWeight.Data);TxCodeData+=4;
					switch (mWeight.Resolution)
					{
						
						case 1: *(TxCodeData) = 0; break;
						case 4: *(TxCodeData) = 2; break;
						case 5: *(TxCodeData) = 3; break;
						case 6: *(TxCodeData) = 4; break;
						default: *(TxCodeData) = 1; break;
					}						
					TxCodeData++;			
					*(TxCodeData) = (uint8_t)(mWeight.Stable);TxCodeData++;		
			Len = 6;
			CommandOut = CMD_TCP_ACK_WEIGHT;
			} else
			{
				Len = 0;
				CommandOut=CMD_TCP_NACK_WEIGHT;
			}
		  break;
			// Запрос состава весов
		case CMD_TCP_GET_SCALE_IDS:
				for (i=0;i<3;i++)
				{
					switch (i)
					{
						case 0: 
							t_I32=mInfo_DLC.Dev_ID;
							t_I64=mInfo_DLC.Dev_SN;	
						break;
						case 1: 
							t_I32=mInfo_CU.Dev_ID;
							t_I64=mInfo_CU.Dev_SN;	
						break;
						case 2: 
							t_I32=mInfo_IU.Dev_ID;
							t_I64=mInfo_IU.Dev_SN;	
						break;
					}
					//IDN
				*(TxCodeData) = (uint8_t)(t_I32>>16);TxCodeData++;
				*(TxCodeData) = (uint8_t)(t_I32>>8);TxCodeData++;
				*(TxCodeData) = (uint8_t)(t_I32);TxCodeData++;
			//SerialNum
				*(TxCodeData) = (uint8_t)(t_I64>>32);TxCodeData++;
				*(TxCodeData) = (uint8_t)(t_I64>>24);TxCodeData++;
				*(TxCodeData) = (uint8_t)(t_I64>>16);TxCodeData++;
				*(TxCodeData) = (uint8_t)(t_I64>>8);TxCodeData++;
				*(TxCodeData) = (uint8_t)(t_I64);TxCodeData++;
				}		
				CommandOut = CMD_TCP_ACK_SCALE_IDS;
				Len=25;
			break;
		case	CMD_TCP_SET_SCALE_NAME:// Задать имя и описание весов
				if (Lenght==0)
				{
							Len=0;
					CommandOut = CMD_TCP_ERROR;
				} 
				else
				{
					tmp=0;k=0;
					for (i=0;i<Lenght;i++)
					{
						if ((*(RxDecodeData+i)==0x0d)||(*(RxDecodeData+i)==0x0a))
						{
							tmp=1;
							k=0;
						} else
						{
							if (tmp==0)
							{
								mAllStoredData.ScaleName[k++]=*(RxDecodeData+i);
							}
							else
							{
							  mAllStoredData.ScaleDescription[k++]=*(RxDecodeData+i);
							}
  					}
					}
					//strncpy((char*)mAllStoredData.ScaleName,(char*)RxDecodeData,Lenght);
					if (StoreNewData()==0)
					{
							CommandOut = CMD_TCP_ACK_SET;
						Len=0;
					}
					else
					{
						 *TxCodeData = TCP_ERROR_SAVE;
							CommandOut = CMD_TCP_ERROR;
								Len=1;
					}

				}
		  break;	
		case CMD_TCP_SET_WIFI:
			
				k=0;
				mAllStoredData.WIFI_IP_ADDR = SetDataToU32(RxDecodeData+k);k+=4;
				mAllStoredData.WIFI_IP_MASK = SetDataToU32(RxDecodeData+k);k+=4;
				mAllStoredData.WIFI_IP_GATE = SetDataToU32(RxDecodeData+k);k+=4;
					mAllStoredData.WIFI_IP_PORT=(uint16_t)(*(RxDecodeData+k));k++;
				mAllStoredData.WIFI_IP_PORT += ((uint16_t)(*(RxDecodeData+k))<<8);k++;
				tmp=0;i=0;
				while ((tmp<4)||(k<Lenght))
				{
						if ((*(RxDecodeData+k)==0x0d)||(*(RxDecodeData+k)==0x0a))
								{
									tmp++;
									i=0;
								} else	
								{	
										if (tmp<2)
										{
											mAllStoredData.WIFI_SSID[i++]=*(RxDecodeData+k);
										}
										else
										{
											mAllStoredData.WIFI_PASSWORD[i++]=*(RxDecodeData+k);
										}
								}
							k++;	
				}

  				if (StoreNewData()==0)
					{
							CommandOut = CMD_TCP_ACK_SET;
							ESP_Reset();
						Len=0;
					}
					else
					{
						 *TxCodeData = TCP_ERROR_SAVE;
							CommandOut = CMD_TCP_ERROR;
								Len=1;
					}

							
					break;
	case CMD_TCP_SET_ETHERNET:
			
	k=0;
			mAllStoredData.ETH_IP_ADDR = SetDataToU32(RxDecodeData+k);k+=4;
				mAllStoredData.ETH_IP_MASK = SetDataToU32(RxDecodeData+k);k+=4;
				mAllStoredData.ETH_IP_GATE = SetDataToU32(RxDecodeData+k);k+=4;
	
				mAllStoredData.ETH_IP_PORT=(uint16_t)(*(RxDecodeData+k));k++;
				mAllStoredData.ETH_IP_PORT += ((uint16_t)(*(RxDecodeData+k))<<8);k++;
	
				if (StoreNewData()==0)
				{
							CommandOut = CMD_TCP_ACK_SET;
							ethernet_interface_init();
								Len=0;
				}
					else
					{
						 *TxCodeData = TCP_ERROR_SAVE;
							CommandOut = CMD_TCP_ERROR;
								Len=1;
					}
		
			break;
  case CMD_TCP_GET_ETHERNET:
					SetDataFromU32(TxCodeData,mAllStoredData.ETH_IP_ADDR);TxCodeData+=4;	
					SetDataFromU32(TxCodeData,mAllStoredData.ETH_IP_MASK);TxCodeData+=4;	
					SetDataFromU32(TxCodeData,mAllStoredData.ETH_IP_GATE);TxCodeData+=4;	
					*(TxCodeData) = (uint8_t)mAllStoredData.ETH_IP_PORT;TxCodeData++;
					*(TxCodeData) = (uint8_t)(mAllStoredData.ETH_IP_PORT>>8);TxCodeData++;
					Len=14;
					CommandOut = CMD_TCP_ACK_GET_ETHERNET;
	   break;
  case CMD_TCP_GET_WIFI:
					
					SetDataFromU32(TxCodeData,mAllStoredData.WIFI_IP_ADDR);TxCodeData+=4;	
					SetDataFromU32(TxCodeData,mAllStoredData.WIFI_IP_MASK);TxCodeData+=4;	
					SetDataFromU32(TxCodeData,mAllStoredData.WIFI_IP_GATE);TxCodeData+=4;	
					*(TxCodeData) = (uint8_t)mAllStoredData.WIFI_IP_PORT;TxCodeData++;
					*(TxCodeData) = (uint8_t)(mAllStoredData.WIFI_IP_PORT>>8);TxCodeData++;
					k=14;
					i=0;
					while (mAllStoredData.WIFI_SSID[i]!=0)
					{
						*(TxCodeData) = mAllStoredData.WIFI_SSID[i];
						TxCodeData++;
						i++;k++;
					}
					*(TxCodeData) = 0x0D;TxCodeData++;k++;
					*(TxCodeData) = 0x0A;TxCodeData++;k++;
				i=0;
					while (mAllStoredData.WIFI_PASSWORD[i]!=0)
					{
						*(TxCodeData) = mAllStoredData.WIFI_PASSWORD[i];
						TxCodeData++;
						i++;k++;
					}
					*(TxCodeData) = 0x0D;TxCodeData++;k++;
					*(TxCodeData) = 0x0A;TxCodeData++;k++;
					
					
					Len=k;
					CommandOut = CMD_TCP_ACK_GET_WIFI;
			break;
	case CMD_TCP_GET_SCALE_INFO:
					//IDN
				*(TxCodeData) = (uint8_t)(BLOCK_VERSIOIN>>16);TxCodeData++;
				*(TxCodeData) = (uint8_t)(BLOCK_VERSIOIN>>8);TxCodeData++;
				*(TxCodeData) = (uint8_t)(BLOCK_VERSIOIN);TxCodeData++;
			//SerialNum
				*(TxCodeData) = (uint8_t)(BLOCK_SERIAL_NUM>>32);TxCodeData++;
				*(TxCodeData) = (uint8_t)(BLOCK_SERIAL_NUM>>24);TxCodeData++;
				*(TxCodeData) = (uint8_t)(BLOCK_SERIAL_NUM>>16);TxCodeData++;
				*(TxCodeData) = (uint8_t)(BLOCK_SERIAL_NUM>>8);TxCodeData++;
				*(TxCodeData) = (uint8_t)(BLOCK_SERIAL_NUM);TxCodeData++;
					//Date
				*(TxCodeData) = (uint8_t)(BLOCK_DATE>>16);TxCodeData++;
				*(TxCodeData) = (uint8_t)(BLOCK_DATE>>8);TxCodeData++;
				*(TxCodeData) = (uint8_t)(BLOCK_DATE);TxCodeData++;
				//CalCode
					SetDataFromU32(TxCodeData,mInfo_DLC.ControlCode);TxCodeData+=4;
				// Широта
				*(TxCodeData) = mInfo_DLC.Latitude;TxCodeData++;
				// Интерфейс
				*(TxCodeData) =CurrentInterface;TxCodeData++;
				// Имя блока
				k=17;
					i=0;
					while (BLOCK_NAME[i]!=0)
					{
						*(TxCodeData) = BLOCK_NAME[i];
						TxCodeData++;
						i++;k++;
					}
					*(TxCodeData) = 0x0D;TxCodeData++;k++;
					*(TxCodeData) = 0x0A;TxCodeData++;k++;
				i=0;
					//Имя весов
					while (mAllStoredData.ScaleName[i]!=0)
					{
						*(TxCodeData) = mAllStoredData.ScaleName[i];
						TxCodeData++;
						i++;k++;
					}
					*(TxCodeData) = 0x0D;TxCodeData++;k++;
					*(TxCodeData) = 0x0A;TxCodeData++;k++;
				i=0;
					//Comment весов
					while (mAllStoredData.ScaleDescription[i]!=0)
					{
						*(TxCodeData) = mAllStoredData.ScaleDescription[i];
						TxCodeData++;
						i++;k++;
					}
					*(TxCodeData) = 0x0D;TxCodeData++;k++;
					*(TxCodeData) = 0x0A;TxCodeData++;k++;
					Len=k;
					lll=Len;
					CommandOut = CMD_TCP_ACK_SCALE_INFO;

			
		break;
	case CMD_TCP_GET_STATE: 
				*TxCodeData=0x1F;
				if (Is_ESP_Work()>0) *TxCodeData |= 0x20;
				TxCodeData++;
				*TxCodeData=0x03;TxCodeData++;
				*TxCodeData=(mWeight.Led_NET<<1)+mWeight.Let_Zero;TxCodeData++;
				*(TxCodeData) =CurrentInterface;
					Len=4;
				CommandOut = CMD_TCP_ACK_GET_STATE;
		break;
	case CMD_TCP_GET_WIFI_SSIDS:
		if ((uint8_t)STORE_LAP[0]==0xff)
		{
								if (ESP_LAP()>0)
								{
										*TxCodeData = TCP_ERROR_ESP_START;
								} else
								{
									*TxCodeData = TCP_ERROR_ESP_BUSY;
								}
								CommandOut = CMD_TCP_ERROR;
								Len=1;
		}
		else
		{
			sprintf((char*)TxCodeData,STORE_LAP);
			Len=strlen(STORE_LAP);
			StoreNewData();

			CommandOut = CMD_TCP_ACK_WIFI_SSIDS;
		}
		
	
		break;
	default:
			Len=0;
			CommandOut = CMD_TCP_NACK;
		break;
		
	}
	

	return CodeData(CommandOut,pChar,Len);
}


int CodeData( unsigned char Command,unsigned char *TxCodeData, int Lenght)
{
	uint16_t crc;

	int i;
	if (Lenght>0)
	{
		for (i=Lenght-1;i>=0;i--)
		{
			*(TxCodeData+i+6)=*(TxCodeData+i);
		}
  }
	
	*TxCodeData=0xf8;
	*(TxCodeData+1)= 0x55;
	*(TxCodeData+2)= 0xce;
	*(TxCodeData+3)=(uint8_t)(Lenght+1);
	*(TxCodeData+4)=(uint8_t)((Lenght+1)>>8);

	*(TxCodeData+5)=Command;
	
	crc= CRC16( TxCodeData+5,Lenght+1);
	*(TxCodeData+Lenght+6)=(unsigned char )crc;
	*(TxCodeData+Lenght+7)=(unsigned char )(crc>>8);	
	
	return Lenght+8;
}


