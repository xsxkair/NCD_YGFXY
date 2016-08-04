/******************************************************************************************/
/*****************************************头文件*******************************************/

#include	"LunchPage.h"

#include	"LCD_Driver.h"

#include	"Define.h"
#include	"MyMem.h"

#include	"SystemSetPage.h"
#include	"SelectUserPage.h"
#include	"PaiDuiPage.h"
#include	"SleepPage.h"
#include	"WifiFunction.h"
#include	"Net_Data.h"
#include	"PlaySong_Task.h"
#include	"UI_Data.h"
#include	"MyTest_Data.h"

#include 	"FreeRTOS.h"
#include 	"task.h"
#include 	"queue.h"

#include	<string.h>
#include	"stdio.h"

/******************************************************************************************/
/*****************************************局部变量声明*************************************/
static LunchPageBuffer * S_LunchPageBuffer = NULL;
/******************************************************************************************/
/*****************************************局部函数声明*************************************/
static void Input(unsigned char *pbuf , unsigned short len);
static void PageUpDate(void);
static MyState_TypeDef PageInit(void *  parm);
static MyState_TypeDef PageBufferMalloc(void);
static MyState_TypeDef PageBufferFree(void);

static void DspPageText(void);
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

unsigned char DspLunchPage(void *  parm)
{
	SetGBSysPage(DspLunchPage, NULL, NULL, Input, PageUpDate, PageInit, PageBufferMalloc, PageBufferFree);
	
	GBPageInit(parm);
	
	return 0;
}


static void Input(unsigned char *pbuf , unsigned short len)
{
	unsigned short *pdata = NULL;
	unsigned char error = 0;
	
	pdata = MyMalloc((len/2)*sizeof(unsigned short));
	if(pdata == NULL)
		return;
	
	/*命令*/
	pdata[0] = pbuf[4];
	pdata[0] = (pdata[0]<<8) + pbuf[5];
	
	/*设置*/
	if(pdata[0] == 0x1b04)
	{
		GBPageBufferFree();
		SetGBChildPage(DspSystemSetPage);
		GotoGBChildPage(NULL);
	}
	/*常规测试*/
	else if(pdata[0] == 0x1b00)
	{	
		error = CreateANewTest(0);
		/*创建成功*/
		if(0 == error)
		{
			GBPageBufferFree();
			SetGBChildPage(DspSelectUserPage);
			GotoGBChildPage(NULL);
		}
		/*禁止常规测试*/
		else if(1 == error)
		{
			SendKeyCode(2);
			AddNumOfSongToList(45, 0);
		}
		/*创建失败*/
		else if(3 == error)
		{
			SendKeyCode(3);
			AddNumOfSongToList(41, 0);
		}
	}
	else if(pdata[0] == 0x1b01)
	{
		if(S_LunchPageBuffer)
			S_LunchPageBuffer->presscount = 0;
	}
	else if(pdata[0] == 0x1b02)
	{
		if(S_LunchPageBuffer)
			S_LunchPageBuffer->presscount++;
	}
	else if(pdata[0] == 0x1b03)
	{
		if(S_LunchPageBuffer)
		{
			/*查看排队状态*/
			if(S_LunchPageBuffer->presscount > 20)
			{
				GBPageBufferFree();
				SetGBChildPage(DspPaiDuiPage);
				GotoGBChildPage(NULL);
			}
			/*批量测试*/
			else
			{
				error = CreateANewTest(111);
				/*创建成功*/
				if(0 == error)
				{
					GBPageBufferFree();
					SetGBChildPage(DspSelectUserPage);
					GotoGBChildPage(NULL);
				}
				/*排队位置满，不允许*/
				else if(1 == error)
				{
					SendKeyCode(1);
					AddNumOfSongToList(40, 0);
				}
				/*创建失败*/
				else if(2 == error)
				{
					SendKeyCode(3);
					AddNumOfSongToList(41, 0);
				}
				/*有卡即将测试*/
				else if(3 == error)
				{
					SendKeyCode(4);
					AddNumOfSongToList(61, 0);
				}
			}
		}
	}
	
	MyFree(pdata);
}

static void PageUpDate(void)
{
	if(TimeOut == timer_expired(&(S_LunchPageBuffer->timer)))
	{
		PageBufferFree();
		DspSleepPage(NULL);
	}
}

static MyState_TypeDef PageInit(void *  parm)
{
	if(My_Fail == PageBufferMalloc())
		return My_Fail;
	
	SelectPage(52);
	
	timer_set(&(S_LunchPageBuffer->timer), 30);
	
	DspPageText();
	
	return My_Pass;
}

static MyState_TypeDef PageBufferMalloc(void)
{
	if(NULL == S_LunchPageBuffer)
	{
		S_LunchPageBuffer = MyMalloc(sizeof(LunchPageBuffer));
		if(S_LunchPageBuffer)
		{
			memset(S_LunchPageBuffer, 0, sizeof(LunchPageBuffer));
			return My_Pass;
		}
	}
	
	return My_Fail;
}

static MyState_TypeDef PageBufferFree(void)
{
	MyFree(S_LunchPageBuffer);
	S_LunchPageBuffer = NULL;
	return My_Pass;
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
static void DspPageText(void)
{
	if(S_LunchPageBuffer)
	{
		memset(S_LunchPageBuffer->buf, 0, 100);
		sprintf(S_LunchPageBuffer->buf, "V %d.%d.%02d", GB_SoftVersion_1, GB_SoftVersion_2, GB_SoftVersion_3);
		DisText(0x1b10, S_LunchPageBuffer->buf, strlen(S_LunchPageBuffer->buf));
	}
}

