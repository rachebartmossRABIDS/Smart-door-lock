#include "stm32f10x.h"                  // Device header

#include "sys.h"
#include "Delay.h"
#include "Timer.h"
#include "Store.h"
#include "usart2.h"
#include "as608.h"

#include "OLED.h"
#include "Stepper.h"
#include "MatrixKey.h"
#include "Buzzer.h"

#include "gizwits_product.h"
#include "gizwits_protocol.h"
#include "usart.h"
#include "usart3.h"
extern u8  sm05receivebuf[16];
extern u8  sm05_OK;
//SysTemPat sys;
u8 DisFlag = 1;
/*
    4 * 4矩阵键盘按键功能定义
    |   1  | |   2  | |   3  | |   4  |
    |   5  | |   6  | |   7  | |   8  |
    |   9  | |   0  | | 退格 | | 确认 |
    | 取消 | | 重置 | | 设置 | | 复位 |
*/

#define Password_Length                 16    //密码长度
#define CorrectPassword         Store_Data    //正确密码数组
#define Cursor                 Password[0]    //当前输入位置，范围：1 ~ Password_Length

#define usart2_baund  57600//串口2波特率，根据指纹模块波特率更改

uint8_t Password[Password_Length + 1];       //输入密码数组，位置0存储光标位置
uint8_t KeyNum;                              //按键键码
uint8_t Mode = 0;                            //模式0：密码输入，模式1：密码设置
uint8_t Open = 0;                            //锁状态，0锁住，1打开
uint8_t LockState = 0;   // ??????
uint8_t ErrorCount = 0;
uint8_t Lock_Flag = 0;       // 锁死标志 0正常 1锁死
uint32_t Lock_Time = 0;      // 锁死开始时间
uint8_t Alarm_Count = 0;
uint32_t openTime = 0;   // 记录开锁时间
SysPara AS608Para;//指纹模块AS608参数
u16 ValidN;//模块内有效指纹个数
uint16_t GET_NUM(void);
uint8_t UI_State = 0;   // 0主界面 1功能菜单
uint8_t Auth = 0;   // 0未认证 1已认证（已开锁）
uint8_t UI_Init = 0;
int Error;  //密码验证信息

void UI_Main(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "智能门锁系统", OLED_8X16);
    OLED_ShowString(0, 16, "1:开锁", OLED_8X16);
    OLED_ShowString(0, 32, "2:管理", OLED_8X16);
    OLED_Update();
}
void UI_Menu(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "管理菜单", OLED_8X16);
    OLED_ShowString(0, 16, "1:录入指纹", OLED_8X16);
    OLED_ShowString(0, 32, "2:删除指纹", OLED_8X16);
    OLED_ShowString(0, 48, "3:改密码", OLED_8X16);
    OLED_ShowString(64, 48, "4:返回", OLED_8X16);
    OLED_Update();
}
/**
  * @brief  密码重置函数，重置密码为"000000"
  * @param  无
  * @retval 无
  */
void Password_Reset(void)
{
    uint8_t i;
	
    for(i = 1; i <= Password_Length; i++)
	{
		if(i <= 6)  {CorrectPassword[i] = '0';}
		else        {CorrectPassword[i] = ' ';}
	}
	Store_Save();
}

/**
  * @brief  输入密码显示函数
  * @param  无
  * @retval 无
  */
void Password_Show(void)
{
    uint8_t i;
	
    for(i = 1; i <= Password_Length; i++)
    {
        OLED_ShowChar((i - 1) * 8, 16, Password[i], OLED_8X16);
    }
    if(Mode == 0)
    {
        OLED_ShowString(0, 48, "                ", OLED_8X16);
    }
}

/**
  * @brief  输入密码清空函数
  * @param  无
  * @retval 无
  */
void Password_Clear(void)
{
    uint8_t i;
	
    for(i = 1; i <= Password_Length; i++)
    {
        Password[i] = ' ';
    	OLED_ShowChar((i - 1) * 8, 16, Password[i], OLED_8X16);
    }
    Cursor = 1;
}

/**
  * @brief  密码比对函数
  * @param  无
  * @retval 返回正确与否，1正确，0错误
  */
uint8_t Password_Match(void)
{
    uint8_t i, Flag = 1;
	
    for(i = 1; i <= Password_Length; i++)
    {
        if(Password[i] != CorrectPassword[i])
		{
			Flag = 0; break;
		}
    }
    return Flag;
}

void ShowErrMessage(uint8_t ensure)
{
    Buzzer_Alarm(1);

    OLED_ShowString(0, 48, (char*)EnsureMessage(ensure), OLED_8X16);
    OLED_Update();

    Delay_ms(1000);

    OLED_ShowString(0, 48, "                ", OLED_8X16);
    OLED_Update();
}

//刷指纹
int press_FR(void)
{
    SearchResult search;
    uint8_t ensure;

    OLED_Clear();
    OLED_ShowString(0, 0, "请输入密码/指纹：.", OLED_8X16);
    OLED_Update();

    // 等待手指按下
    while (1)
    {
        ensure = PS_GetImage();

        if (ensure == 0x00)
        {
            break;
        }
        else if (ensure == 0x02)
        {
            Delay_ms(50);
            continue;
        }
        else
        {
            OLED_ShowString(0, 16, "传感器错误", OLED_8X16);
            OLED_Update();
            return -1;
        }
    }

    OLED_ShowString(0, 16, "采集中...", OLED_8X16);
    OLED_Update();

    // 生成特征
    ensure = PS_GenChar(CharBuffer1);
    if (ensure != 0x00)
    {
        OLED_ShowString(0, 32, "特征失败", OLED_8X16);
        OLED_Update();
        return -1;
    }

    // 搜索
    ensure = PS_HighSpeedSearch(CharBuffer1, 0,100 , &search);
    if (ensure == 0x00)
    {
        OLED_ShowString(0, 48, "匹配成功!", OLED_8X16);
    }
    else
    {
        OLED_ShowString(0, 48, "匹配失败!", OLED_8X16);
    }

    OLED_Update();
    Delay_ms(1000);

    return ensure == 0x00 ? 0 : -1;
}

void Add_FR(void)
{
    uint8_t ensure;
    uint16_t id;

    OLED_Clear();
    OLED_ShowString(0, 0, "录入指纹", OLED_8X16);
    OLED_Update();

    // ===== 第一次采集 =====
    OLED_ShowString(0, 16, "按手指", OLED_8X16);
    while(PS_GetImage() != 0x00);

    ensure = PS_GenChar(CharBuffer1);
    if(ensure != 0x00)
    {
        ShowErrMessage(ensure);
        return;
    }

    OLED_ShowString(0, 32, "第一次OK", OLED_8X16);
    OLED_Update();
    Delay_ms(1000);

    // ===== 第二次采集 =====
    OLED_ShowString(0, 16, "再按一次", OLED_8X16);
    while(PS_GetImage() != 0x00);

    ensure = PS_GenChar(CharBuffer2);
    if(ensure != 0x00)
    {
        ShowErrMessage(ensure);
        return;
    }

    OLED_ShowString(0, 32, "第二次OK", OLED_8X16);
    OLED_Update();
    Delay_ms(1000);

    // ===== 比对 =====
    ensure = PS_Match();
    if(ensure != 0x00)
    {
        OLED_ShowString(0, 48, "不一致", OLED_8X16);
        OLED_Update();
        return;
    }

    // ===== 生成模板 =====
    ensure = PS_RegModel();
    if(ensure != 0x00)
    {
        ShowErrMessage(ensure);
        return;
    }

    // ===== 输入ID =====
    id = GET_NUM();
    if(id == 0xFFFF) return;

    // ===== 存储 =====
    ensure = PS_StoreChar(CharBuffer2, id);

    char buf[20];
    sprintf(buf, "store=%02X", ensure);
    OLED_ShowString(0, 48, buf, OLED_8X16);
    OLED_Update();

    if(ensure == 0x00)
    {
        OLED_ShowString(0, 32, "录入成功!", OLED_8X16);
    }
    else
    {
        ShowErrMessage(ensure);
    }

    OLED_Update();
    Delay_ms(1500);
}

void Del_FR(void)
{
    uint16_t id;
    uint8_t ensure;

    id = GET_NUM();
    if(id == 0xFFFF) return;

    ensure = PS_DeletChar(id, 1);

    if(ensure == 0x00)
    {
        OLED_ShowString(0,32,"删除成功",OLED_8X16);
    }
    else
    {
        ShowErrMessage(ensure);
    }

    OLED_Update();
    Delay_ms(1500);
}

uint16_t GET_NUM(void)
{
    uint16_t num = 0;
    uint8_t key;
    OLED_Clear();
    OLED_ShowString(0, 0, "输入ID:", OLED_8X16);
    OLED_ShowString(0, 48, "12确认 16返回", OLED_8X16);
    OLED_Update();

    while(1)
    {
        key = MatrixKey_GetNum();
			
        if(key != 0)
        {
             Delay_ms(20);
             while(MatrixKey_GetNum());   // 等松手
            if(key >= 1 && key <= 10)
            {
                uint8_t digit = key % 10;
                num = num * 10 + digit;

                OLED_ShowString(0, 16, "ID:", OLED_8X16);
                OLED_ShowNum(24, 16, num, 5, OLED_8X16);
                OLED_Update();
            }
                        // ===== 退格 =====
            else if(key == 11)
            {
                num /= 10;

                OLED_ShowString(0, 16, "ID:     ", OLED_8X16);
                OLED_ShowNum(24, 16, num, 5, OLED_8X16);
                OLED_Update();
            }

            // ===== 按确认（第一次）=====
            else if(key == 12 )
            {
                OLED_ShowString(0, 32, "已确认ID", OLED_8X16);
                OLED_ShowString(0, 48, "16返回主菜单", OLED_8X16);
                OLED_Update();
							return num;
            }
            // ===== 直接返回（未确认）=====
            else if(key == 16 )
            {
                return 0xFFFF;
            }
        }
				Delay_ms(50);
    }
}

void Buzzer_Beep_N(uint8_t times)
{
    uint8_t i;
    for(i = 0; i < times; i++)
    {
        Buzzer_On();
        Delay_ms(150);
        Buzzer_Off();
        Delay_ms(150);
    }
}

void Error_Handle(void)
{
    ErrorCount++;
    Alarm_Count++;
currentDataPoint.valueAlarm_Count = Alarm_Count;
    // 普通错误 → 响3声
    if(ErrorCount < 3)
    {
        Buzzer_Beep_N(3);
    }
    else
    {
        // 超过3次 → 锁死
        Lock_Flag = 1;
			Open = 0; 
        Lock_Time = gizGetTimerCount();

        Buzzer_Beep_N(5);   // 锁死报警5声

        OLED_Clear();
        OLED_ShowString(0, 0, "系统错误!", OLED_8X16);
        OLED_ShowString(0, 16, "锁30s按16返回", OLED_8X16);
        OLED_Update();
    }
}

int main(void)
{
  //uint8_t i;
	if(Store_Init())    //初始化正确密码为"000000"
	{
		Password_Reset();
	}
  OLED_Init();
	MatrixKey_Init();
	Stepper_Init();
	Timer_Init();
	Buzzer_Init();
	
	USART1_Config();//串口初始化
	USART3_Config();//串口初始化
	TIM3_Int_Init(1000-1,72-1);		//定时1ms中断
	
	usart2_init(usart2_baund);   // 初始化指纹模块串口
  PS_StaGPIO_Init();            // 初始化指纹触摸
	Open = 0;       
  LockState = 0;  
  Buzzer_Off();
	
  userInit();
	gizwitsInit();
  //gizwitsSetMode(WIFI_AIRLINK_MODE);	
	
  UI_Main();
	Password_Clear();	
while(1)
{ 
	  // ===== 锁死状态处理 =====
if(Lock_Flag)
{
    KeyNum = MatrixKey_GetNum();

    // ===== 按16立即退出 =====
    if(KeyNum == 16)
    {
        while(MatrixKey_GetNum());   // 等松手

        Lock_Flag = 0;
        ErrorCount = 0;
        KeyNum = 0;

        UI_State = 0;
        UI_Init = 0;
        UI_Main();
    }

    // ===== 30秒自动恢复 =====
    if(gizGetTimerCount() - Lock_Time > 30000)
    {
        Lock_Flag = 0;
        ErrorCount = 0;

        OLED_Clear();
        OLED_ShowString(0,0,"ok",OLED_8X16);
        OLED_Update();
        Delay_ms(1000);

        UI_State = 0;
        UI_Init = 0;
        UI_Main();
    }

    continue;
}
	  userHandle();
	  //gizwitsHandle();
		gizwitsHandle((dataPoint_t *)&currentDataPoint);
    currentDataPoint.valueAlarm_Count = Alarm_Count;
    KeyNum = MatrixKey_GetNum();

    // ================= 主界面 =================
    if(UI_State == 0)
    {
        static uint8_t init = 0;
        if(init == 0)
        {
            UI_Main();
            init = 1;
        }

        if(KeyNum == 1)
        {
            UI_State = 1;   // 开锁界面
            OLED_Clear();
            OLED_ShowString(0,0,"请输入密码:",OLED_8X16);
            Password_Clear();
            OLED_Update();
        }
        else if(KeyNum == 2)
        {
            if(Auth == 1)
            {
                UI_State = 2;   // 管理菜单
                UI_Menu();
            }
            else
            {
                OLED_Clear();
                OLED_ShowString(0,0,"请先开锁!",OLED_8X16);
                OLED_Update();
                Delay_ms(1000);
                UI_Main();
            }
        }
				else if(KeyNum == 3)
{
    if(Open == 1)
    {
        Open = 0;          // 关键：触发关锁
        Auth = 0;          // 退出认证状态（建议加）
        openTime = 0;

        OLED_Clear();
        OLED_ShowString(0,0,"ok",OLED_8X16);
        OLED_Update();
        Delay_ms(1000);

        UI_Main();
    }
    else
    {
        OLED_Clear();
        OLED_ShowString(0,0,"已锁",OLED_8X16);
        OLED_Update();
        Delay_ms(1000);

        UI_Main();
    }
}
    }

    // ================= 开锁界面 =================
    else if(UI_State == 1)
    {
        // ?? 指纹识别
        if(PS_Sta)
        {
            Error = press_FR();
            while(PS_Sta);

            if(Error == 0)
            {
                OLED_ShowString(0,48,"指纹正确!",OLED_8X16);
                Buzzer_Beep(100);
                Open = 1;
							//openTime = 0;
                Auth = 1;
                ErrorCount = 0;
                UI_State = 0;
                UI_Main();
            }
            else
            {
                OLED_ShowString(0,48,"指纹错误!",OLED_8X16);
							//Alarm_Count++;
							Error_Handle();
            }
        }

        // ?? 键盘输入
        if(KeyNum)
        {
            if(KeyNum <= 10)   // 数字
            {
                if(Cursor <= Password_Length)
                {
                    Password[Cursor++] = KeyNum % 10 + '0';
                    Password_Show();
                }
            }
            else if(KeyNum == 11)   // 退格
            {
                if(Cursor > 1)
                {
                    Password[--Cursor] = ' ';
                    Password_Show();
                }
            }
            else if(KeyNum == 12)   // 确认
            {
                if(Password_Match())
                {
                    OLED_ShowString(0,48,"密码正确!",OLED_8X16);
                    Buzzer_Beep(100);
                    Open = 1;
									//openTime = 0;
                    Auth = 1;
									ErrorCount = 0;
                    UI_State = 0;
                    UI_Main();
                }
                else
                {
                    OLED_ShowString(0,48,"密码错误!",OLED_8X16);
									//Alarm_Count++;
									Error_Handle();
                }
            }
            else if(KeyNum == 14)   // 重置密码
            {
                Password_Clear();
                Password_Reset();
                OLED_ShowString(0,48,"密码已重置!",OLED_8X16);
                OLED_Update();
            }
            else if(KeyNum == 16)   // 返回
            {
                UI_State = 0;
                UI_Main();
            }
        }
    }

    // ================= 管理菜单 =================
    else if(UI_State == 2)
    {
			//UI_Menu();
        static uint8_t init = 0;
        if(init == 0)
        {
            UI_Menu();
            init = 1;
        }

        if(KeyNum == 1)
        {
            init = 0; 
					Add_FR();   // ?只在这里允许
            //UI_Menu();
        }
        else if(KeyNum == 2)
        {
            init = 0; 
					Del_FR();
            //UI_Menu();
        }
        else if(KeyNum == 3)
        {
            UI_State = 3;   // 修改密码状态
            OLED_Clear();
            OLED_ShowString(0,0,"输入新密码:",OLED_8X16);
            Password_Clear();
            Mode = 1;
            OLED_Update();
        }
        else if(KeyNum == 4 || KeyNum == 16)
        {
					init = 0; 
            UI_State = 0;
            UI_Main();
        }
    }

    // ================= 修改密码 =================
    else if(UI_State == 3)
    {
        if(KeyNum)
        {
            if(KeyNum <= 10)
            {
                if(Cursor <= Password_Length)
                {
                    Password[Cursor++] = KeyNum % 10 + '0';
                    Password_Show();
                }
            }
            else if(KeyNum == 11)
            {
                if(Cursor > 1)
                {
                    Password[--Cursor] = ' ';
                    Password_Show();
                }
            }
            else if(KeyNum == 12)
            {
                uint8_t i;
                for(i = 1; i <= Password_Length; i++)
                {
                    CorrectPassword[i] = Password[i];
                }
                Store_Save();

                OLED_ShowString(0,48,"密码设置成功!",OLED_8X16);
                OLED_Update();
                Delay_ms(1000);

                Mode = 0;
                UI_State = 2;
                UI_Menu();
            }
            else if(KeyNum == 16)
            {
                Mode = 0;
                UI_State = 2;
                UI_Menu();
            }
        }
    }

    // ================= 电机控制 =================
    if(Open && LockState == 0)
    {
        Stepper_Move(1024, 1, 2);
        LockState = 1;
    }
    else if(!Open && LockState == 1)
    {
        Stepper_Move(1024, 0, 2);
        LockState = 0;
    }
if(Open && openTime == 0)
{
    openTime = gizGetTimerCount();
}

if(Open && (gizGetTimerCount() - openTime > 5000))
{
    Open = 0;
    openTime = 0;
}
    OLED_Update();
}
}

void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)    //判断是否为定时器TIM2更新中断
    {
        MatrixKey_Tick();
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);    //清除定时器TIM2更新中断标志位			
    }
}
