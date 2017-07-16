#include "main.h"
#include "stm32f0xx_hal.h"
#include "usb_device.h"
#include "I2C.h"
#include "USB.h"
#include "types_defines.h"
//#include "ifs_ethernet.h"
#include "coders.h"
#include "rs232.h"


uint32_t Address = 0, PageError = 0;
__IO uint32_t data32 = 0 , MemoryProgramStatus = 0;

/*Variable used for Erase procedure*/
static FLASH_EraseInitTypeDef EraseInitStruct;
int FlashEraseData(void);


int FlashReadData(uint32_t* mStoreData)
{
	int i;
	for (i=0;i<sizeof(mAllStoredData)/4;i++)
	{
		*(mStoreData+i)=(uint32_t)STORE_DATA_AREA[i];
	}
	return 0;
}

uint32_t Addres, dData;;
int FlashStoreData(uint8_t* mStoreData)
{
	int i;
	
	HAL_FLASH_Unlock();
	
	if (FlashEraseData()<0) return -1;
	
	for (i=0;i<sizeof(mAllStoredData)/4;i++)
	{
//		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,(uint32_t)(STORE_DATA_AREA+i*4),*(mStoreData+i*4));
		Addres=(uint32_t)(STORE_DATA_AREA+i);
		dData=(uint32_t)(*((uint32_t*)(mStoreData+i*4)));
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,(uint32_t)(Addres),dData);
	}
	
	HAL_FLASH_Lock();
	return 0;
}

int FlashStoreLAP(char* mStoreData)
{ 
	int i;
		HAL_FLASH_Unlock();
		for (i=0;i<=strlen((char*)mStoreData)/4;i++)
	{
//		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,(uint32_t)(STORE_DATA_AREA+i*4),*(mStoreData+i*4));
		Addres=(uint32_t)(STORE_LAP+i*4);
		dData=(uint32_t)(*((uint32_t*)(mStoreData+i*4)));
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,(uint32_t)(Addres),dData);
	}
	
	HAL_FLASH_Lock();
	return 0;
}



int FlashEraseData(void)
{
 EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = (uint32_t)STORE_DATA_AREA;
  EraseInitStruct.NbPages = 1;

	  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
			return -1;
	else
		return 0;
		
}




