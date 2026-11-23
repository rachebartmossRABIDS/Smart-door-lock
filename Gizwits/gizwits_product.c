#include <stdio.h>
#include <string.h>
#include "gizwits_product.h"
#include "common.h"
#include "usart.h"	
#include "usart3.h"
static uint32_t timerMsCount;
uint8_t aRxBuffer;
dataPoint_t currentDataPoint;
extern uint8_t LockState;
extern uint8_t Alarm_Count;
extern uint8_t Open;
int8_t gizwitsEventProcess(eventInfo_t *info, uint8_t *gizdata, uint32_t len)
{
    uint8_t i = 0;
    dataPoint_t *dataPointPtr = (dataPoint_t *)gizdata;
    moduleStatusInfo_t *wifiData = (moduleStatusInfo_t *)gizdata;
    protocolTime_t *ptime = (protocolTime_t *)gizdata;
    
#if MODULE_TYPE
    gprsInfo_t *gprsInfoData = (gprsInfo_t *)gizdata;
#else
    moduleInfo_t *ptModuleInfo = (moduleInfo_t *)gizdata;
#endif

    if((NULL == info) || (NULL == gizdata))
    {
        return -1;
    }

    for(i=0; i<info->num; i++)
    {
        switch(info->event[i])
        {
        case EVENT_Lock_Control:
            currentDataPoint.valueLock_Control = dataPointPtr->valueLock_Control;
            GIZWITS_LOG("Evt: EVENT_Lock_Control %d \n", currentDataPoint.valueLock_Control);
            /*if(0x01 == currentDataPoint.valueLock_Control)
            {
            //Open = 0;//user handle
            }
            else
            {
            //Open = 1;//user handle    
            }*/
				    if(currentDataPoint.valueLock_Control == 1)
    {
        Open = 1;           // ¿ªËø
    }
    else
    {
        Open = 0;           // ¹ØËø
    }
            break;
        case EVENT_Alarm_Clear:
            currentDataPoint.valueAlarm_Clear = dataPointPtr->valueAlarm_Clear;
            GIZWITS_LOG("Evt: EVENT_Alarm_Clear %d \n", currentDataPoint.valueAlarm_Clear);
            /*if(0x01 == currentDataPoint.valueAlarm_Clear)
            {
            //user handle
            }
            else
            {
            //user handle    
            }*/
    if(currentDataPoint.valueAlarm_Clear == 1)
    {
        Alarm_Count = 0;
        currentDataPoint.valueAlarm_Count = 0;
        //gizwitsHandle((dataPoint_t *)&currentDataPoint);
    }
            break;




        case WIFI_SOFTAP:
            break;
        case WIFI_AIRLINK:
            break;
        case WIFI_STATION:
            break;
        case WIFI_CON_ROUTER:
 
            break;
        case WIFI_DISCON_ROUTER:
 
            break;
        case WIFI_CON_M2M:
 
            break;
        case WIFI_DISCON_M2M:
            break;
        case WIFI_RSSI:
            GIZWITS_LOG("RSSI %d\n", wifiData->rssi);
            break;
        case TRANSPARENT_DATA:
            GIZWITS_LOG("TRANSPARENT_DATA \n");
            //user handle , Fetch data from [data] , size is [len]
            break;
        case WIFI_NTP:
            GIZWITS_LOG("WIFI_NTP : [%d-%d-%d %02d:%02d:%02d][%d] \n",ptime->year,ptime->month,ptime->day,ptime->hour,ptime->minute,ptime->second,ptime->ntp);
            break;
        case MODULE_INFO:
            GIZWITS_LOG("MODULE INFO ...\n");
#if MODULE_TYPE
            GIZWITS_LOG("GPRS MODULE ...\n");
            //Format By gprsInfo_t
            GIZWITS_LOG("moduleType : [%d] \n",gprsInfoData->Type);
#else
            GIZWITS_LOG("WIF MODULE ...\n");
            //Format By moduleInfo_t
            GIZWITS_LOG("moduleType : [%d] \n",ptModuleInfo->moduleType);
#endif
        break;
        default:
            break;
        }
    }

    return 0;
}

/**
* User data acquisition

* Here users need to achieve in addition to data points other than the collection of data collection, can be self-defined acquisition frequency and design data filtering algorithm

* @param none
* @return none
*/
void userHandle(void)
{
	currentDataPoint.valueAlarm_Clear = 0;
    currentDataPoint.valueLock_State = LockState;//Add Sensor Data Collection
    currentDataPoint.valueAlarm_Count = Alarm_Count;//Add Sensor Data Collection


    
}

/**
* Data point initialization function

* In the function to complete the initial user-related data
* @param none
* @return none
* @note The developer can add a data point state initialization value within this function
*/
void userInit(void)
{
    memset((uint8_t*)&currentDataPoint, 0, sizeof(dataPoint_t));
    
    /** Warning !!! DataPoint Variables Init , Must Within The Data Range **/ 
    
    currentDataPoint.valueLock_Control = 0;
    currentDataPoint.valueLock_State = 0;
    currentDataPoint.valueAlarm_Clear =0 ;
    currentDataPoint.valueAlarm_Count = 0;
    

}


/**
* @brief Millisecond timing maintenance function, milliseconds increment, overflow to zero

* @param none
* @return none
*/
void gizTimerMs(void)
{
    timerMsCount++;
}

/**
* @brief Read millisecond count

* @param none
* @return millisecond count
*/
uint32_t gizGetTimerCount(void)
{
    return timerMsCount;
}

/**
* @brief MCU reset function

* @param none
* @return none
*/
void mcuRestart(void)
{
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
int32_t uartWrite(uint8_t *buf, uint32_t len)
{
		//uint8_t crc[1] = {0x55};
    uint32_t i = 0;
	
    if(NULL == buf)
    {
        return -1;
    }

for(i=0; i<len; i++)
{
USART_SendData(USART3, buf[i]);
while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
if(i >=2 && buf[i] == 0xFF)
{
USART_SendData(USART3,0x55);
while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}
}

#ifdef PROTOCOL_DEBUG
    GIZWITS_LOG("MCU2WiFi[%4d:%4d]: ", gizGetTimerCount(), len);
    for(i=0; i<len; i++)
    {
        GIZWITS_LOG("%02x ", buf[i]);

        if(i >=2 && buf[i] == 0xFF)
        {
            GIZWITS_LOG("%02x ", 0x55);
        }
    }
    GIZWITS_LOG("\n");
#endif
		
		return len;
}  
