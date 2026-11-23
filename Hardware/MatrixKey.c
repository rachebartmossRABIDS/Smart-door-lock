#include "stm32f10x.h"                  // Device header

/*引脚配置*/
#define R1        GPIO_Pin_4    //PA7
#define R2        GPIO_Pin_5    //PA6
#define R3        GPIO_Pin_6    //PA5
#define R4        GPIO_Pin_7    //PA4
#define C1        GPIO_Pin_11    //PA3
#define C2        GPIO_Pin_12    //PA2
#define C3        GPIO_Pin_1    //PA1
#define C4        GPIO_Pin_0    //PA0

uint8_t MatrixKey_Num;

/**
  * @brief  矩阵键盘初始化函数，初始化PA0 ~ PA3为列输入，PA4 ~ PA7为行输出，即逐行扫描模式
  * @param  无
  * @retval 无
  */
void MatrixKey_Init(void)
{
	//配置RCC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	//配置GPIO
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //推挽输出模式
	GPIO_InitStructure.GPIO_Pin = R1 | R2 | R3 | R4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      //上拉输入模式
	GPIO_InitStructure.GPIO_Pin = C1 | C2 | C3 | C4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA, R1 | R2 | R3 | R4);
}

/**
  * @brief  获取矩阵键盘键码函数
  * @param  无
  * @retval 返回键码值，若无按键按下则返回0
  */
uint8_t MatrixKey_GetNum(void)
{
	uint8_t Temp;
	
	Temp = MatrixKey_Num;
	MatrixKey_Num = 0;
	return Temp;
}

/**
  * @brief  获取矩阵键盘按键状态函数
  * @param  无
  * @retval 返回键码值，若无按键按下则返回0
  */
uint8_t MatrixKey_GetState(void)
{
	uint8_t KeyNum = 0;
	
	GPIO_ResetBits(GPIOA, R1);
	GPIO_SetBits(GPIOA, R2 | R3 | R4);
	if(GPIO_ReadInputDataBit(GPIOA, C1) == 0)  {KeyNum = 1;}
	if(GPIO_ReadInputDataBit(GPIOA, C2) == 0)  {KeyNum = 2;}
	if(GPIO_ReadInputDataBit(GPIOA, C3) == 0)  {KeyNum = 3;}
	if(GPIO_ReadInputDataBit(GPIOA, C4) == 0)  {KeyNum = 4;}
	
	GPIO_ResetBits(GPIOA, R2);
	GPIO_SetBits(GPIOA, R1 | R3 | R4);
	if(GPIO_ReadInputDataBit(GPIOA, C1) == 0)  {KeyNum = 5;}
	if(GPIO_ReadInputDataBit(GPIOA, C2) == 0)  {KeyNum = 6;}
	if(GPIO_ReadInputDataBit(GPIOA, C3) == 0)  {KeyNum = 7;}
	if(GPIO_ReadInputDataBit(GPIOA, C4) == 0)  {KeyNum = 8;}
	
	GPIO_ResetBits(GPIOA, R3);
	GPIO_SetBits(GPIOA, R1 | R2 | R4);
	if(GPIO_ReadInputDataBit(GPIOA, C1) == 0)  {KeyNum = 9;}
	if(GPIO_ReadInputDataBit(GPIOA, C2) == 0)  {KeyNum = 10;}
	if(GPIO_ReadInputDataBit(GPIOA, C3) == 0)  {KeyNum = 11;}
	if(GPIO_ReadInputDataBit(GPIOA, C4) == 0)  {KeyNum = 12;}
	
	GPIO_ResetBits(GPIOA, R4);
	GPIO_SetBits(GPIOA, R1 | R2 | R3);
	if(GPIO_ReadInputDataBit(GPIOA, C1) == 0)  {KeyNum = 13;}
	if(GPIO_ReadInputDataBit(GPIOA, C2) == 0)  {KeyNum = 14;}
	if(GPIO_ReadInputDataBit(GPIOA, C3) == 0)  {KeyNum = 15;}
	if(GPIO_ReadInputDataBit(GPIOA, C4) == 0)  {KeyNum = 16;}
	
	GPIO_SetBits(GPIOA, R1 | R2 | R3 | R4);
	
	return KeyNum;
}

/**
  * @brief  矩阵键盘定时中断函数，由main中的1ms定时中断调用
  * @param  无
  * @retval 无
  */
void MatrixKey_Tick(void)
{
	static uint8_t Count;
	static uint8_t NowState, LastState;
	
	Count++;
	if(Count >= 20)    //每隔20ms执行一次
	{
		Count = 0;
		
		LastState = NowState;
		NowState = MatrixKey_GetState();
		if(LastState != 0 && NowState == 0)
		{
			MatrixKey_Num = LastState;
		}
	}
}
