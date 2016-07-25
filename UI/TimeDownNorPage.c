/******************************************************************************************/
/*****************************************ͷ�ļ�*******************************************/

#include	"TimeDownNorPage.h"
#include	"Define.h"
#include	"LCD_Driver.h"
#include	"UI_Data.h"
#include	"SystemSetPage.h"
#include	"MyMem.h"
#include	"TestPage.h"
#include	"MyTest_Data.h"

#include 	"FreeRTOS.h"
#include 	"task.h"
#include 	"queue.h"

#include	<string.h>
#include	"stdio.h"

/******************************************************************************************/
/*****************************************�ֲ���������*************************************/
static TimeDownPageData *S_TimeDownPageData = NULL;

/******************************************************************************************/
/*****************************************�ֲ���������*************************************/
static void RefreshTimeText(void);
static void Input(unsigned char *pbuf , unsigned short len);
static void PageUpData(void);

static MyState_TypeDef PageInit(void *  parm);
static MyState_TypeDef PageBufferMalloc(void);
static MyState_TypeDef PageBufferFree(void);
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

unsigned char DspTimeDownNorPage(void *  parm)
{
	SysPage * myPage = GetSysPage();   

	myPage->CurrentPage = DspTimeDownNorPage;
	myPage->LCDInput = Input;
	myPage->PageUpData = PageUpData;
	myPage->ParentPage = NULL;
	myPage->ChildPage = DspTestPage;
	myPage->PageInit = PageInit;
	myPage->PageBufferMalloc = PageBufferMalloc;
	myPage->PageBufferFree = PageBufferFree;
	
	myPage->PageInit(parm);
	
	SelectPage(65);
	
	return 0;
}


static void Input(unsigned char *pbuf , unsigned short len)
{
	unsigned short *pdata = NULL;
	
	pdata = MyMalloc((len/2)*sizeof(unsigned short));
	if(pdata == NULL)
		return;
	
	/*����*/
	pdata[0] = pbuf[4];
	pdata[0] = (pdata[0]<<8) + pbuf[5];
	

	
	MyFree(pdata);
}

static void PageUpData(void)
{
	static unsigned char count = 0;
	
	if(count % 50 == 0)
	{
		RefreshTimeText();
		if((S_TimeDownPageData)&&(TimeOut == timer_expired(S_TimeDownPageData->S_Timer)))
		{
			PageBufferFree();
			GetSysPage()->ChildPage(NULL);
		}
	}
	
	count++;
}

static MyState_TypeDef PageInit(void *  parm)
{
	if(My_Fail == PageBufferMalloc())
		return My_Fail;
	
	S_TimeDownPageData->currenttestdata = GetCurrentTestItem();
	
	S_TimeDownPageData->S_Timer = &(S_TimeDownPageData->currenttestdata->timer);
	
	return My_Pass;
}

static MyState_TypeDef PageBufferMalloc(void)
{	
	S_TimeDownPageData = (TimeDownPageData *)MyMalloc(sizeof(TimeDownPageData));
			
	if(S_TimeDownPageData)
	{
		memset(S_TimeDownPageData, 0, sizeof(TimeDownPageData));
		
		return My_Pass;
	}
	else
		return My_Fail;
}

static MyState_TypeDef PageBufferFree(void)
{
	MyFree(S_TimeDownPageData);
	S_TimeDownPageData = NULL;
	
	return My_Pass;
}

static void RefreshTimeText(void)
{
	char *buf = NULL;
	buf = MyMalloc(50);
	if(buf && S_TimeDownPageData)
	{
		memset(buf, 0, 50);
		
		ClearText(0x2000, 30);
		sprintf(buf, "%d", timer_surplus(S_TimeDownPageData->S_Timer));
		DisText(0x2000, buf, strlen(buf));
	}
	
	MyFree(buf);
}



