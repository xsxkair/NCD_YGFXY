/***************************************************************************************************
*FileName：SystemUI_Task
*Description：系统界面更新任务
*Author：xsx
*Data：2016年4月30日11:21:29
***************************************************************************************************/

/***************************************************************************************************/
/******************************************头文件***************************************************/
/***************************************************************************************************/

#include	"SystemUI_Task.h"
#include	"UI_Data.h"
#include	"LCD_Driver.h"
#include	"PaiDuiPage.h"

#include 	"FreeRTOS.h"
#include 	"task.h"

#include	"stdio.h"
#include	"string.h"

/***************************************************************************************************/
/**************************************局部变量声明*************************************************/
/***************************************************************************************************/

#define SystemUITask_PRIORITY			2			//看门狗任务优先级
const char * SystemUITaskName = "vSystemUITask";		//看门狗任务名
/***************************************************************************************************/
/**************************************局部函数声明*************************************************/
/***************************************************************************************************/

static void vSystemUITask( void *pvParameters );	//看门狗任务

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************正文********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

/***************************************************************************************************
*FunctionName：StartvSystemUITask
*Description：建立系统指示灯任务
*Input：None
*Output：None
*Author：xsx
*Data：2015年8月26日16:58:13
***************************************************************************************************/
void StartvSystemUITask(void)
{
	xTaskCreate( vSystemUITask, SystemUITaskName, configMINIMAL_STACK_SIZE*2, NULL, SystemUITask_PRIORITY, NULL );
}

/***************************************************************************************************
*FunctionName：vSysLedTask
*Description：系统指示灯闪烁表面程序正常运行
*Input：None
*Output：None
*Author：xsx
*Data：2015年8月26日16:58:46
***************************************************************************************************/
static void vSystemUITask( void *pvParameters )
{
	static unsigned int count = 0;
	PageInfo * currentpage = NULL;
	
	while(1)
	{
		if(My_Pass == GetCurrentPage(&currentpage))
		{
			if(NULL != currentpage->PageUpDate)
				currentpage->PageUpDate();
		}
		
		/*500ms更新一次时间和环境温度*/
		if(count % 10 == 0)
			DspTimeAndTempData();
		
		if(count % 100 == 0)
			CheckTime();
		
		count++;
		
		vTaskDelay(10 / portTICK_RATE_MS);
	}
}
