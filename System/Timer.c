#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "timer.h"
#include "string.h"
#include "gizwits_product.h"
#include "usart.h"	
/**
  * @brief  定时器初始化函数，初始化TIM2为使用内部时钟的1ms计时器
  * @param  无
  * @retval 无
  */
	extern vu16 USART2_RX_STA;
void Timer_Init(void)
{
    //配置RCC
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);    //通用计时器TIM2
    
    //选择时钟
    TIM_InternalClockConfig(TIM2);    //内部时钟
    
    //配置时基单元，标准频率1kHz，Feq = 72M / (PSC + 1)
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;        //不分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;    //向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;                   //ARR自动重装器值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;                  //PSC预分频器值
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;               //重复计数器值（高级定时器）
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
    
    //配置中断
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);          //防止复位后立刻进入中断，清除中断标志位
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);     //使能TIM2更新中断
    
    //配置NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //分组2：2位抢占优先级，2位响应优先级
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;    //抢占优先级2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;           //响应优先级1
    NVIC_Init(&NVIC_InitStructure);
    
    //使能能定时器
    TIM_Cmd(TIM2, ENABLE);
}

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM3, ENABLE);  //使能TIMx					 
}
//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
			        if(USART2_RX_STA != 0)   // ? 必须判断是否真的收到数据
        {
            USART2_RX_STA |= 1 << 15;   // 标记一帧完成
        }
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
				gizTimerMs();
			//TIM_Cmd(TIM3, DISABLE);  //关闭TIM3
		}
}
