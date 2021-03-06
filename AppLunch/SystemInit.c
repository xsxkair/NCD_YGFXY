/***************************************************************************************************
*FileName:SystemInit
*Description: 系统所有模块初始化
*Author : xsx
*Data: 2016年4月21日14:18:28
***************************************************************************************************/

/***************************************************************************************************/
/******************************************头文件***************************************************/
/***************************************************************************************************/
#include	"SystemInit.h"

#include	"Led_Driver.h"
#include 	"Usart3_Driver.h"
#include 	"Usart2_Driver.h"
#include 	"Usart1_Driver.h"
#include	"Ads8325_Driver.h"
#include	"DRV8825_Driver.h"
#include	"CodeScanner_Driver.h"
#include	"user_fatfs.h"
#include 	"MLX90614_Driver.h"
#include	"RTC_Driver.h"
#include 	"Usart6_Driver.h"
#include 	"Iwdg_Driver.h"
#include	"CardLimit_Driver.h"
#include 	"Usart4_Driver.h"
#include	"Wifi_Driver.h"
#include	"Timer4_Driver.h"
#include	"Timer3_Driver.h"
#include	"PT8211_Driver.h"
#include	"TM1623_Driver.h"
#include	"MAX4051_Driver.h"
#include	"TLV5617_Driver.h"
#include	"LEDCheck_Driver.h"
#include	"DS18b20_Driver.h"

#include 	"usbd_hid_core.h"
#include 	"usbd_usr.h"
#include 	"usbd_desc.h"
#include 	"usb_conf.h"

#include	"Delay.h"
/***************************************************************************************************/
/**************************************局部变量声明*************************************************/
/***************************************************************************************************/
#if (USB_USE == 1)
USB_OTG_CORE_HANDLE USB_OTG_dev;
#endif				//USB_USE 

/***************************************************************************************************/
/**************************************局部函数声明*************************************************/
/***************************************************************************************************/
extern void SystemInit(void);
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************正文********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

/***************************************************************************************************
*FunctionName：MySystemInit
*Description：系统初始化
*Input：None
*Output：None
*Author：xsx
*Data：2016年4月21日14:20:44
***************************************************************************************************/
void MySystemBSPInit(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	delay_ms(500);
	SystemInit();						//手动系统初始化
	delay_ms(500);
	
	Led_GPIOInit();						//系统指示灯初始化
	delay_ms(1);
	
	Timer4_Init();						//定时器初始化
	Timer3_Init();						//定时器初始化
	delay_ms(1);
	
	Usart3_Init();						//串口3初始化
	delay_ms(1);
	
	Usart2_Init();						//串口2初始化
	delay_ms(1);
	
	Usart4_Init();						//串口4初始化
	delay_ms(1);
	
	Usart6_Init();						//串口6初始化
	delay_ms(1);
	
	Usart1_Init();						//串口2初始化
	delay_ms(1);

	ADS_IO_Init();						//ADC IO初始化
	delay_ms(1);
	
	DA_IO_Init();						//DA io初始化
	delay_ms(1);
	
	Limit_Init();						//限位开关初始化
	delay_ms(1);
	
	DRV_Init();							//电机 IO初始化
	delay_ms(1);
	
	CodeScanner_GPIOInit();				//二维码初始化
	delay_ms(1);
	
	SMBus_Init();						//温度检测初始化
	delay_ms(1);
	
	RTC_BSPInit();						//RTC初始化
	delay_ms(1);
	
	Wifi_GPIOInit();					//WIFI端口初始化
	delay_ms(1);
	
	MAX4051_Init();						//采集放大倍数端口初始化
	delay_ms(1);
	
	PT8211_GPIOInit();					//音频端口初始化
	ConfigI2S(16000);
	delay_ms(1);

	TM1623_Init();						//排队模块初始化
	delay_ms(1);
	
	LEDCheck_Init();					//检测发光模块的发光二极管状态初始化
	delay_ms(1);

	DS18B20_Init();						//初始化18b20
	delay_ms(1);
	
	FatfsInit();						//文件系统初始化
	
	#if (USB_USE == 1)
		USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_HID_cb, &USR_cb);
	#endif				//USB_USE 
	
	//IWDG_Init(3, 3000);					//看门狗初始化,超时时间2S
	delay_ms(1);
}

