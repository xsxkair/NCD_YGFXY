/***************************************************************************************************
*FileName：Universal_Task
*Description：通用任务
*Author：xsx
*Data：2016年5月6日16:56:17
***************************************************************************************************/

/***************************************************************************************************/
/******************************************头文件***************************************************/
/***************************************************************************************************/

#include	"Universal_Task.h"
#include 	"MLX90614_Driver.h"
#include	"Led_Driver.h"

#include	"OutModel_Fun.h"
#include	"Universal_Fun.h"

#include	"Motor_Data.h"
#include	"CardStatues_Data.h"

#include 	"FreeRTOS.h"
#include 	"task.h"

#include	"stdio.h"

/***************************************************************************************************/
/**************************************局部变量声明*************************************************/
/***************************************************************************************************/

#define UniversalTask_PRIORITY			2			//看门狗任务优先级
const char * UniversalTaskName = "vUniversalTask";		//看门狗任务名
/***************************************************************************************************/
/**************************************局部函数声明*************************************************/
/***************************************************************************************************/

static void vUniversalTask( void *pvParameters );	//看门狗任务

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************正文********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

/***************************************************************************************************
*FunctionName：StartvSysLedTask
*Description：建立系统指示灯任务
*Input：None
*Output：None
*Author：xsx
*Data：2015年8月26日16:58:13
***************************************************************************************************/
char StartvUniversalTask(void)
{
	return xTaskCreate( vUniversalTask, UniversalTaskName, configMINIMAL_STACK_SIZE, NULL, UniversalTask_PRIORITY, NULL );
}

/***************************************************************************************************
*FunctionName：vSysLedTask
*Description：系统指示灯闪烁表面程序正常运行
*Input：None
*Output：None
*Author：xsx
*Data：2015年8月26日16:58:46
***************************************************************************************************/
static void vUniversalTask( void *pvParameters )
{
	static unsigned int count = 0;
	
	while(1)
	{
		/*读取时间，500ms采集一次*/
		if(count % 5 == 0)
			UpDateGB_Time();
		
		/*检测卡状态，500ms间隔*/
		if(count % 5 == 0)
			CheckCardStatues();
		
		/*系统状态灯*/
		LedToggle();
		
		//读取环境温度
		if(count % 2 == 0)
			ReadEnvironmentTemperature();
		
		//控制排队模块状态
		ChangeOutModelStatues();
		
		count++;
		
		vTaskDelay(100 / portTICK_RATE_MS);
	}
}
