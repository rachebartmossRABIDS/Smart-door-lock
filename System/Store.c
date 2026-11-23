#include "stm32f10x.h"                  // Device header
#include "MyFLASH.h"

#define STORE_START_ADDRESS        0x0800FC00
#define STORE_COUNT                512
#define STORE_INIT_FLAG            0xA5A3

uint16_t Store_Data[STORE_COUNT];

/**
  * @brief  用户数据存储区初始化函数，初始化FLASH最后一页为用户数据存储区
  * @param  无
  * @retval 返回是否第一次初始化，第一次返回1，否则返回0
  */
uint8_t Store_Init(void)
{
	uint8_t  Flag = 0;
	uint16_t i;

	if(MyFLASH_ReadHalfWord(STORE_START_ADDRESS) != STORE_INIT_FLAG)
	{
		MyFLASH_ErasePages(STORE_START_ADDRESS);
		MyFLASH_ProgramHalfWord(STORE_START_ADDRESS, STORE_INIT_FLAG);
		for(i = 1; i < STORE_COUNT; i++)
		{
			MyFLASH_ProgramHalfWord(STORE_START_ADDRESS + i * 2, 0x0000);
		}
		Flag = 1;
	}

	for(i = 0; i < STORE_COUNT; i++)
	{
		Store_Data[i] = MyFLASH_ReadHalfWord(STORE_START_ADDRESS + i * 2);
	}
	
	return Flag;
}

/**
  * @brief  用户数据存储区保存函数
  * @param  无
  * @retval 无
  */
void Store_Save(void)
{
	uint16_t i;
	
	MyFLASH_ErasePages(STORE_START_ADDRESS);
	for(i = 0; i < STORE_COUNT; i++)
	{
		MyFLASH_ProgramHalfWord(STORE_START_ADDRESS + i * 2, Store_Data[i]);
	}
}

/**
  * @brief  用户数据存储区清空函数
  * @param  无
  * @retval 无
  */
void Store_Clear(void)
{
	uint16_t i;
	
	for(i = 1; i < STORE_COUNT; i++)
	{
		Store_Data[i] = 0x0000;
	}
	Store_Save();
}
