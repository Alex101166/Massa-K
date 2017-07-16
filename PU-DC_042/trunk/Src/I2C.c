
#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include <stdbool.h>


extern I2C_HandleTypeDef hi2c1;
uint8_t* I2C_DecodeData(uint8_t* pData,int Len);
extern	uint8_t rxI2CCount;
int32_t I2C_ReadDataID(uint8_t addr);
int64_t I2C_ReadDataSN(uint8_t addr);

uint8_t I2C_Buffer[20];
uint8_t I2C_Out_Buffer[20];

uint8_t WeightStatusTimer;

// Удалить после отладки

//uint16_t aDataBuffer[10];
//uint8_t aAddrBuffer[10];
//uint16_t cDataBuffer;
uint8_t I2C_Status,I2C_Error;
uint16_t I2C_TimerCount;
//int IFE[100];
//int iii;
//bool DataInputFlag;
uint8_t CommandFlag;

//до сюда

int count;


int I2C_Init(void)
{
	count=0;

	I2C_TimerCount=0;
	CommandFlag=0;
	WeightStatusTimer=0;

	
		return 0;
}

void I2C_Timer(void)
{
	// Чтение информации о блоках
	switch (I2C_TimerCount)
	{
		case 40:		CommandFlag=1; break;// ID DLS
		case 42:		CommandFlag=2; break;// SN DLC
		case 44:		CommandFlag=3; break;// ControlCode
		case 46:		CommandFlag=4; break;// ID_CU
		case 48:		CommandFlag=5; break;// SN CU
		case 50:		CommandFlag=6; break;// ID IU
		case 52:		CommandFlag=7; break;// SN IU

		//	mInfo_DLC.Dev_ID=I2C_ReadDataID(0x08);
	}
	if (I2C_TimerCount <250) I2C_TimerCount++;
	
	// Определение обновления веса
	if (WeightStatusTimer++ > 10)
		mWeight.Change=false;
}

int Result;

	uint8_t I2C_Buffer_TM[6];


int32_t I2C_ReadDataID(uint8_t addr)
{
	int32_t Res;
	I2C_Buffer_TM[0]=0x50;
	I2C_Buffer_TM[1]=0x01;
	I2C_Buffer_TM[2]=0x00;
	
		while 	(HAL_I2C_GetState(&hi2c1)!=HAL_I2C_STATE_READY);
	Result = HAL_I2C_Master_Transmit(&hi2c1,(uint16_t)(addr),I2C_Buffer_TM,3,200);
	if (Result!= HAL_OK)
	{
		    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler();
    }
		I2C_Status = HAL_I2C_GetState(&hi2c1);
		return -2;
	}
	
	while 	(HAL_I2C_GetState(&hi2c1)!=HAL_I2C_STATE_READY);
	
	 Result = HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(addr), (uint8_t *)I2C_Buffer_TM, 4,200);
	if ( Result	!= HAL_OK)
  {
    /* Error_Handler() function is called when Timeout error occurs.
       When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler();
    }
		I2C_Status = HAL_I2C_GetState(&hi2c1);
		return -1;
  }

	Res=(int32_t)(((uint32_t)(I2C_Buffer_TM[1]<<16))+((uint32_t)(I2C_Buffer_TM[2]<<8))+((uint32_t)I2C_Buffer_TM[3]));
	return Res;
}

int32_t I2C_ReadControlCode(uint8_t addr)
{
	int32_t Res;
	I2C_Buffer_TM[0]=0x50;
	I2C_Buffer_TM[1]=0x11;
	I2C_Buffer_TM[2]=0x27;
	
	while 	(HAL_I2C_GetState(&hi2c1)!=HAL_I2C_STATE_READY);
	
	Result = HAL_I2C_Master_Transmit(&hi2c1,(uint16_t)(addr),I2C_Buffer_TM,3,200);
	if (Result!= HAL_OK)
	{
		    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler();
    }
		I2C_Status = HAL_I2C_GetState(&hi2c1);
		return -2;
	}
	
	while 	(HAL_I2C_GetState(&hi2c1)!=HAL_I2C_STATE_READY);
	
	 Result = HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(addr), (uint8_t *)I2C_Buffer_TM, 5,200);
	if ( Result	!= HAL_OK)
  {
    /* Error_Handler() function is called when Timeout error occurs.
       When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler();
    }
		I2C_Status = HAL_I2C_GetState(&hi2c1);
		return -1;
  }

	Res=(int32_t)(((uint32_t)(I2C_Buffer_TM[1]<<24))+((uint32_t)(I2C_Buffer_TM[1]<<16))+((uint32_t)(I2C_Buffer_TM[2]<<8))+((uint32_t)I2C_Buffer_TM[3]));
	return Res;
}

int64_t I2C_ReadDataSN(uint8_t addr)
{
	int64_t Res;
	I2C_Buffer_TM[0]=0x50;
	I2C_Buffer_TM[1]=0x01;
	I2C_Buffer_TM[2]=0x01;
	
		while 	(HAL_I2C_GetState(&hi2c1)!=HAL_I2C_STATE_READY);
	Result = HAL_I2C_Master_Transmit(&hi2c1,(uint16_t)(addr),I2C_Buffer_TM,3,200);
	if (Result!= HAL_OK)
	{
		    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler();
    }
		I2C_Status = HAL_I2C_GetState(&hi2c1);
		return -2;
	}
	
	while 	(HAL_I2C_GetState(&hi2c1)!=HAL_I2C_STATE_READY);
	
	 Result = HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(addr), (uint8_t *)I2C_Buffer_TM, 6,200);
	if ( Result	!= HAL_OK)
  {
    /* Error_Handler() function is called when Timeout error occurs.
       When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler();
    }
		I2C_Status = HAL_I2C_GetState(&hi2c1);
		return -1;
  }

	Res=(int64_t)((((uint64_t)I2C_Buffer_TM[1]) <<32)+(((uint64_t)I2C_Buffer_TM[2])<<24)+
	(((uint64_t)I2C_Buffer_TM[3])<<16)+(((uint64_t)I2C_Buffer_TM[4])<<8)+((uint64_t)I2C_Buffer_TM[5]));
	return Res;
}


uint8_t outbuf[10];
//int ISR;
//int addr;
int Data_Out_Count;

int I2C_Pool(void)
{
	
		if	(HAL_I2C_GetState(&hi2c1)==HAL_I2C_STATE_READY)
		{
	
			if (CommandFlag>0)
			{
				switch (CommandFlag)
				{
					case 1:
						mInfo_DLC.Dev_ID=I2C_ReadDataID(0x10);
					break;
				case 2:
						mInfo_DLC.Dev_SN=I2C_ReadDataSN(0x10);
					break;
				case 3:
						mInfo_DLC.ControlCode=I2C_ReadControlCode(0x10);
					break;
				case 4:
						mInfo_CU.Dev_ID=I2C_ReadDataID(0x20);
					break;
				case 5:
						mInfo_CU.Dev_SN=I2C_ReadDataSN(0x20);
					break;
				case 6:
						mInfo_IU.Dev_ID=I2C_ReadDataID(0x30);
					break;
				case 7:
						mInfo_IU.Dev_SN=I2C_ReadDataSN(0x30);
					break;
					
					default:
						break;
				}
				CommandFlag=0;
				
			}
		 else
		 {
				if (HAL_I2C_Slave_Receive_IT(&hi2c1, (uint8_t *)I2C_Buffer, 100) == HAL_OK)
				{
				

				//		Этот блок удалить после отладки
					/*
					if (count<100)
					{	
							if (DataInputFlag)
							{
					  aDataBuffer[count]=(uint16_t)(I2C_Buffer[1]<<8)+ (uint16_t)I2C_Buffer[2];
						aAddrBuffer[count]=I2C_Buffer[0];
							if (aDataBuffer[count]>0x07f0) cDataBuffer  =aDataBuffer[count];
							} else
							{
								aDataBuffer[count]=0xaa55;
							}
							count++;
						
					}
					*/
					// До сюда
					
				I2C_DecodeData(I2C_Buffer,rxI2CCount);
				}
					
							
			//		HAL_GPIO_TogglePin(TEST_POINT_GPIO_Port,TEST_POINT_Pin);
			//		HAL_GPIO_WritePin(TEST_POINT_GPIO_Port,TEST_POINT_Pin,GPIO_PIN_RESET);				
			
				return 1;
				}	
		} 
	
		return 0;
		

}

uint8_t* I2C_DecodeData(uint8_t* pData,int Len)
{
	int i;
	int32_t Data;
		I2C_Out_Buffer[0]= 3;
	switch (pData[1])
	{
				case 1:
				{
						switch (pData[2])
						{
							case 0:
								I2C_Out_Buffer[0]= 1;
								I2C_Out_Buffer[1] = (uint8_t)(BLOCK_VERSIOIN >> 16);
								I2C_Out_Buffer[2] = (uint8_t)(BLOCK_VERSIOIN >> 8);
								I2C_Out_Buffer[3] = (uint8_t)(BLOCK_VERSIOIN);
							break;
							case 1:
								I2C_Out_Buffer[0]= 1;
								I2C_Out_Buffer[1] = (uint8_t)(BLOCK_SERIAL_NUM >> 32);
								I2C_Out_Buffer[2] = (uint8_t)(BLOCK_SERIAL_NUM >> 24);
								I2C_Out_Buffer[3] = (uint8_t)(BLOCK_SERIAL_NUM >> 16);
								I2C_Out_Buffer[4] = (uint8_t)(BLOCK_SERIAL_NUM >> 8);
								I2C_Out_Buffer[5] = (uint8_t)(BLOCK_SERIAL_NUM);
							break;
						}

				}
				break;
				case 7:
						I2C_Out_Buffer[0]=2;
						mWeight.Resolution=(pData[2] & 0x3e) >> 1;
						mWeight.Stable= pData[2] & 0x01;
						mWeight.Let_Zero = (pData[2] & 0x80) >> 7;
						mWeight.Led_NET = (pData[2] & 0x40) >> 6;
						Data=(int32_t)( (uint32_t)pData[3]+((uint32_t)pData[4] << 8 ) + ((uint32_t)( pData[5]  & 0x7f ) << 16 ) );
						if ((pData[5] & 0x80) !=0) 
							Data=-Data;
						mWeight.Data=Data;
						mWeight.Change=true;
						WeightStatusTimer=0;
				break;
				case 6:
				{
					switch (pData[2])
					{
						case 1:
									I2C_Out_Buffer[0]=2;
								for (i=0;i<7;i++)
							{
								mW_Indicator.data[i]=pData[i+3];
								mW_Indicator.Change=true;
							}
							break;
					}
					
				}
				break;
		}
	return I2C_Out_Buffer;
}
