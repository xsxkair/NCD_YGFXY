/******************************************************************************************/
/*****************************************头文件*******************************************/

#include	"RecordPage.h"

#include	"LCD_Driver.h"

#include	"Define.h"
#include	"MyMem.h"

#include	"SystemSetPage.h"
#include	"ShowResultPage.h"
#include	"CRC16.h"
#include	"Printf_Fun.h"
#include	"SDFunction.h"
#include	"UI_Data.h"
#include	"System_Data.h"

#include 	"FreeRTOS.h"
#include 	"task.h"
#include 	"queue.h"

#include	<string.h>
#include	"stdio.h"
#include 	"stdlib.h"

/******************************************************************************************/
/*****************************************局部变量声明*************************************/
static RecordPageBuffer * S_RecordPageBuffer = NULL;
/******************************************************************************************/
/*****************************************局部函数声明*************************************/
static void Input(unsigned char *pbuf , unsigned short len);
static void PageUpDate(void);

static MyState_TypeDef PageInit(void *parm);
static MyState_TypeDef PageBufferMalloc(void);
static MyState_TypeDef PageBufferFree(void);

static MyState_TypeDef ShowRecord(unsigned char pageindex);
static MyState_TypeDef ReadFileSaveDateInfo(void);
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

unsigned char DspRecordPage(void *  parm)
{
	SetGBParentPage(DspSystemSetPage);
	SetGBChildPage(DspShowResultPage);
	SetGBPageUpDate(PageUpDate);
	SetGBGBPageInput(Input);
	SetGBPageInit(PageInit);
	SetGBPageBufferMalloc(PageBufferMalloc);
	SetGBPageBufferFree(PageBufferFree);
	
	if(DspSystemSetPage == GetGBCurrentPage())
		GBPageInit(parm);
	
	SelectPage(86);
	
	SetGBCurrentPage(DspRecordPage);
	
	return 0;
}


static void Input(unsigned char *pbuf , unsigned short len)
{
	if(S_RecordPageBuffer)
	{
		/*命令*/
		S_RecordPageBuffer->lcdinput[0] = pbuf[4];
		S_RecordPageBuffer->lcdinput[0] = (S_RecordPageBuffer->lcdinput[0]<<8) + pbuf[5];
		
		/*返回*/
		if(S_RecordPageBuffer->lcdinput[0] == 0x2801)
		{
			GBPageBufferFree();
			GotoGBParentPage(NULL);
		}
		/*打印*/
		else if(S_RecordPageBuffer->lcdinput[0] == 0x2800)
		{
			if(My_Pass == ConnectPrintter())
			{
				SendKeyCode(1);
				PrintfData(&(S_RecordPageBuffer->testdata[S_RecordPageBuffer->selectindex]));
				SendKeyCode(9);
			}
			else
				SendKeyCode(2);
		}
		/*上一页*/
		else if(S_RecordPageBuffer->lcdinput[0] == 0x2802)
		{
			if(S_RecordPageBuffer->pageindex > 1)
				S_RecordPageBuffer->pageindex -= 1;
			else
				S_RecordPageBuffer->pageindex = S_RecordPageBuffer->maxpagenum;
				
			ShowRecord(S_RecordPageBuffer->pageindex);
		}
		/*下一页*/
		else if(S_RecordPageBuffer->lcdinput[0] == 0x2803)
		{
			if(S_RecordPageBuffer->pageindex < S_RecordPageBuffer->maxpagenum)
				S_RecordPageBuffer->pageindex += 1;
			else
				S_RecordPageBuffer->pageindex = 1;
				
			ShowRecord(S_RecordPageBuffer->pageindex);
		}
		/*选择数据长按 -- 按下*/
		else if((S_RecordPageBuffer->lcdinput[0] >= 0x2805)&&(S_RecordPageBuffer->lcdinput[0] <= 0x280d))
		{
			S_RecordPageBuffer->longpresscount = 0;
			if(S_RecordPageBuffer->testdata[(S_RecordPageBuffer->lcdinput[0] - 0x2805)].crc == CalModbusCRC16Fun1(&(S_RecordPageBuffer->testdata[(S_RecordPageBuffer->lcdinput[0] - 0x2805)]), sizeof(TestData)-2))
			{
				S_RecordPageBuffer->selectindex = (S_RecordPageBuffer->lcdinput[0] - 0x2805);
				BasicPic(0x28c0, 1, 100, 39, 522, 968, 556, 39, 140+(S_RecordPageBuffer->selectindex)*36);
			}
		}
		/*选择数据长按 -- 持续按下*/
		else if(S_RecordPageBuffer->lcdinput[0] == 0x2804)
		{
			S_RecordPageBuffer->longpresscount++;
		}
		/*选择数据长按 -- 松开*/
		else if((S_RecordPageBuffer->lcdinput[0] >= 0x280e)&&(S_RecordPageBuffer->lcdinput[0] <= 0x2816))
		{
			if(S_RecordPageBuffer->longpresscount > 5)
				GotoGBChildPage(&S_RecordPageBuffer->testdata[S_RecordPageBuffer->selectindex]);
		}
		/*选择其他文件*/
		else if(S_RecordPageBuffer->lcdinput[0] == 0x28d0)
		{
			S_RecordPageBuffer->tempvalue1 = strtol((char *)(&pbuf[7]), NULL, 10);
			if( (S_RecordPageBuffer->tempvalue1 > 0) && (S_RecordPageBuffer->tempvalue1 <= S_RecordPageBuffer->maxpagenum))
			{
				S_RecordPageBuffer->pageindex = S_RecordPageBuffer->tempvalue1;
		
				S_RecordPageBuffer->selectindex = 0;

				ShowRecord(S_RecordPageBuffer->pageindex);
			}
		}
	}
}

static void PageUpDate(void)
{

}

static MyState_TypeDef PageInit(void *parm)
{
	if(My_Fail == PageBufferMalloc())
		return My_Fail;
	
	ReadFileSaveDateInfo();
	
	S_RecordPageBuffer->selectindex = 0;
	S_RecordPageBuffer->pageindex = 1;
	ShowRecord(S_RecordPageBuffer->pageindex);

	return My_Pass;
}

static MyState_TypeDef PageBufferMalloc(void)
{	
	if(NULL == S_RecordPageBuffer)
	{
		S_RecordPageBuffer = (RecordPageBuffer *)MyMalloc(sizeof(RecordPageBuffer));
		
		if(NULL == S_RecordPageBuffer)
			return My_Fail;
	}
	
	memset(S_RecordPageBuffer, 0, sizeof(RecordPageBuffer));
	return My_Pass;
}

static MyState_TypeDef PageBufferFree(void)
{
	MyFree(S_RecordPageBuffer);
	S_RecordPageBuffer = NULL;
	
	return My_Pass;
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
static MyState_TypeDef ReadFileSaveDateInfo(void)
{
	if(S_RecordPageBuffer)
	{
		GetTestDataNum(&(S_RecordPageBuffer->maxdatanum));
		
		S_RecordPageBuffer->maxpagenum = ((S_RecordPageBuffer->maxdatanum % DataNumInPage) == 0)?(S_RecordPageBuffer->maxdatanum / DataNumInPage):
		((S_RecordPageBuffer->maxdatanum / DataNumInPage)+1);
	}
	
	return My_Pass;
}

static MyState_TypeDef ShowRecord(unsigned char pageindex)
{
	unsigned short i=0;
	
	if(S_RecordPageBuffer)
	{
		memset(S_RecordPageBuffer->testdata, 0, DataNumInPage*sizeof(TestData));
		
		S_RecordPageBuffer->tempvalue1 = pageindex-1;
		S_RecordPageBuffer->tempvalue1 *= DataNumInPage;
		
		ReadTestData(&(S_RecordPageBuffer->testdata[0]), S_RecordPageBuffer->tempvalue1, DataNumInPage);
		
		BasicPic(0x28c0, 0, 100, 39, 522, 968, 556, 39, 140+(S_RecordPageBuffer->selectindex)*36);
		
		S_RecordPageBuffer->tempdata = &(S_RecordPageBuffer->testdata[0]);
		for(i=0; i<DataNumInPage; i++)
		{
			if(S_RecordPageBuffer->tempdata->crc == CalModbusCRC16Fun1(S_RecordPageBuffer->tempdata, sizeof(TestData)-2))
			{
				memset(S_RecordPageBuffer->buf, 0, 300);
				sprintf(S_RecordPageBuffer->buf, "%5d   %10s%15s  \0", (pageindex-1)*DataNumInPage+i+1, S_RecordPageBuffer->tempdata->temperweima.ItemName,
				S_RecordPageBuffer->tempdata->sampleid);
				
				/*if(S_RecordPageBuffer->tempdata->testline.AdjustResult <= S_RecordPageBuffer->tempdata->temperweima.LowstResult)
					sprintf(S_RecordPageBuffer->buf2, "<%.2f\0", S_RecordPageBuffer->tempdata->temperweima.LowstResult);
				else if(S_RecordPageBuffer->tempdata->testline.AdjustResult >= S_RecordPageBuffer->tempdata->temperweima.HighestResult)
					sprintf(S_RecordPageBuffer->buf2, ">%.2f\0", S_RecordPageBuffer->tempdata->temperweima.HighestResult);
				else*/
					sprintf(S_RecordPageBuffer->buf2, "%.2f\0", S_RecordPageBuffer->tempdata->testline.AdjustResult);
				strcat(S_RecordPageBuffer->buf, S_RecordPageBuffer->buf2);
				
				sprintf(S_RecordPageBuffer->buf2, " %s %d-%d-%d %d:%d:%d %s \0", S_RecordPageBuffer->tempdata->temperweima.ItemMeasure,
				S_RecordPageBuffer->tempdata->TestTime.year, S_RecordPageBuffer->tempdata->TestTime.month, S_RecordPageBuffer->tempdata->TestTime.day,
				S_RecordPageBuffer->tempdata->TestTime.hour, S_RecordPageBuffer->tempdata->TestTime.min, S_RecordPageBuffer->tempdata->TestTime.sec,
				S_RecordPageBuffer->tempdata->user.user_name);
				strcat(S_RecordPageBuffer->buf, S_RecordPageBuffer->buf2);
				
				DisText(0x28e0+(i)*0x30, S_RecordPageBuffer->buf, strlen(S_RecordPageBuffer->buf));
			}
			else
				ClearText(0x28e0+(i)*0x30, 77);
			
			S_RecordPageBuffer->tempdata++;
		}
		
		return My_Pass;
	}
	
	return My_Fail;
}

