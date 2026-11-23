#include "stm32f10x.h"                  // Device header

/**
  * @brief  FLASH读取字函数
  * @param  Address 要读取的起始地址
  * @retval 读取的32位数据
  */
uint32_t MyFLASH_ReadWord(uint32_t Address)
{
	return *((__IO uint32_t *) (Address));
}

/**
  * @brief  FLASH读取半字函数
  * @param  Address 要读取的起始地址
  * @retval 读取的16位数据
  */
uint16_t MyFLASH_ReadHalfWord(uint32_t Address)
{
	return *((__IO uint16_t *) (Address));
}

/**
  * @brief  FLASH读取字节函数
  * @param  Address 要读取的起始地址
  * @retval 读取的8位数据
  */
uint8_t MyFLASH_ReadByte(uint32_t Address)
{
	return *((__IO uint8_t *) (Address));
}

/**
  * @brief  FLASH全擦除函数
  * @param  无
  * @retval 无
  */
void MyFLASH_EraseAllPages(void)
{
	FLASH_Unlock();
	FLASH_EraseAllPages();
	FLASH_Lock();
}

/**
  * @brief  FLASH页擦除函数
  * @param  PageAddress 要擦除的页起始地址
  * @retval 无
  */
void MyFLASH_ErasePages(uint32_t PageAddress)
{
	FLASH_Unlock();
	FLASH_ErasePage(PageAddress);
	FLASH_Lock();
}

/**
  * @brief  FLASH写入字函数
  * @param  Address 要写入的起始地址
  * @param  Data 要写入的32位数据
  * @retval 无
  */
void MyFLASH_ProgramWord(uint32_t Address, uint32_t Data)
{
	FLASH_Unlock();
	FLASH_ProgramWord(Address, Data);
	FLASH_Lock();
}

/**
  * @brief  FLASH写入半字函数
  * @param  Address 要写入的起始地址
  * @param  Data 要写入的16位数据
  * @retval 无
  */
void MyFLASH_ProgramHalfWord(uint32_t Address, uint16_t Data)
{
	FLASH_Unlock();
	FLASH_ProgramHalfWord(Address, Data);
	FLASH_Lock();
}
