/******************************************************************************************/
/*****************************************头文件*******************************************/

#include	"WifiSetPage.h"

#include	"LCD_Driver.h"
#include	"UI_Data.h"
#include	"MyMem.h"
#include	"WifiFunction.h"
#include	"NetPreSetPage.h"
#include	"SDFunction.h"

#include 	"FreeRTOS.h"
#include 	"task.h"
#include 	"queue.h"

#include	<string.h>
#include	"stdio.h"

/******************************************************************************************/
/*****************************************局部变量声明*************************************/
static WifiPageBuffer * S_WifiPageBuffer = NULL;
/******************************************************************************************/
/*****************************************局部函数声明*************************************/
static MyState_TypeDef RefreshWifi(void);
static void DisListText(void);
static void Input(unsigned char *pbuf , unsigned short len);
static void PageUpDate(void);
static void CheckIsNeedKey(void);

static MyState_TypeDef PageInit(void *  parm);
static MyState_TypeDef PageBufferMalloc(void);
static MyState_TypeDef PageBufferFree(void);
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

unsigned char DspWifiSetPage(void *  parm)
{
	PageInfo * currentpage = NULL;
	
	if(My_Pass == GetCurrentPage(&currentpage))
	{
		currentpage->PageInit = PageInit;
		currentpage->PageUpDate = PageUpDate;
		currentpage->LCDInput = Input;
		currentpage->PageBufferMalloc = PageBufferMalloc;
		currentpage->PageBufferFree = PageBufferFree;
		
		currentpage->PageInit(currentpage->pram);
	}
	
	return 0;
}


static void Input(unsigned char *pbuf , unsigned short len)
{
	if(S_WifiPageBuffer)
	{
		/*命令*/
		S_WifiPageBuffer->lcdinput[0] = pbuf[4];
		S_WifiPageBuffer->lcdinput[0] = (S_WifiPageBuffer->lcdinput[0]<<8) + pbuf[5];
		
		/*获得密码连接wifi*/
		if(S_WifiPageBuffer->lcdinput[0] == 0x1E70)
		{
			if(S_WifiPageBuffer)
			{
				SendKeyCode(4);
				vTaskDelay(1000/portTICK_RATE_MS);
				memcpy(S_WifiPageBuffer->wifip->key, &pbuf[7], GetBufLen(&pbuf[7] , 2*pbuf[6]));
				if(My_Fail == ConnectWifi(S_WifiPageBuffer->wifip))
				{
					SendKeyCode(16);
					vTaskDelay(100/portTICK_RATE_MS);
					SendKeyCode(2);
				}
				else
				{
					RestartWifi();
					vTaskDelay(2000/portTICK_RATE_MS);
					SendKeyCode(16);
					vTaskDelay(100/portTICK_RATE_MS);
					SendKeyCode(1);
					SaveWifiData(S_WifiPageBuffer->wifip);
				}
			}
		}
		/*上一页*/
		else if(S_WifiPageBuffer->lcdinput[0] == 0x1E52)
		{
			if(S_WifiPageBuffer)
			{
				if(S_WifiPageBuffer->pageindex > 0)
				{
					S_WifiPageBuffer->pageindex--;
					DisListText();
				}
			}
		}
		/*下一页*/
		else if(S_WifiPageBuffer->lcdinput[0] == 0x1E53)
		{
			if(S_WifiPageBuffer)
			{
				if(S_WifiPageBuffer->pageindex < (MaxWifiListNum/PageWifiNum-1))
				{
					if(strlen(S_WifiPageBuffer->wifilist[(S_WifiPageBuffer->pageindex+1)*PageWifiNum].ssid) > 0)
					{
						S_WifiPageBuffer->pageindex++;
						DisListText();
					}
				}
			}
		}
		/*返回*/
		else if(S_WifiPageBuffer->lcdinput[0] == 0x1E50)
		{
			PageBufferFree();
			
			PageBackTo(ParentPage);
		}
		/*刷新*/
		else if(S_WifiPageBuffer->lcdinput[0] == 0x1E51)
		{
			RefreshWifi();
			DisListText();
		}
		/*选择wifi*/
		else if((S_WifiPageBuffer->lcdinput[0] >= 0x1E58)&&(S_WifiPageBuffer->lcdinput[0] <= 0x1E5F))
		{
			if(S_WifiPageBuffer)
			{
				/*判断选择的wifi是否超出了有效列表*/
				if(strlen(S_WifiPageBuffer->wifilist[(S_WifiPageBuffer->pageindex)*PageWifiNum+(S_WifiPageBuffer->lcdinput[0] - 0x1E58)].ssid) > 0)
				{
					S_WifiPageBuffer->selectindex = (S_WifiPageBuffer->lcdinput[0] - 0x1E58)+1;
					CheckIsNeedKey();
				}
			}
		}
	}
}

static void PageUpDate(void)
{
	
}

static MyState_TypeDef PageInit(void *  parm)
{
	if(My_Fail == PageBufferMalloc())
		return My_Fail;
	
	SelectPage(112);
	
	BasicPic(0x1E70, 0, 111, 13, 569, 253, 599, 392, 124);
	
	RefreshWifi();
	DisListText();
	
	return My_Pass;
}

static MyState_TypeDef PageBufferMalloc(void)
{
	if(NULL == S_WifiPageBuffer)
	{
		S_WifiPageBuffer = MyMalloc(sizeof(WifiPageBuffer));
			
		if(NULL == S_WifiPageBuffer)
			return My_Fail;
	}
	
	memset(S_WifiPageBuffer, 0, sizeof(WifiPageBuffer));
		
	return My_Pass;
}

static MyState_TypeDef PageBufferFree(void)
{
	MyFree(S_WifiPageBuffer);
	S_WifiPageBuffer = NULL;
	
	return My_Pass;
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
static MyState_TypeDef RefreshWifi(void)
{
	if(S_WifiPageBuffer)
	{
		vTaskDelay(100*portTICK_RATE_MS);
		
		S_WifiPageBuffer->pageindex = 0;
		S_WifiPageBuffer->selectindex = 0;
		
		SendKeyCode(5);
	
		/*如果不是at模式，则进入at模式*/
		if(AT_Mode != GetWifiWorkMode())
			SetWifiWorkInAT(AT_Mode);
		
		memset(S_WifiPageBuffer->wifilist, 0, sizeof(WIFI_Def)*MaxWifiListNum);
		
		ScanApList(S_WifiPageBuffer->wifilist);
		vTaskDelay(500*portTICK_RATE_MS);
		
		SendKeyCode(17);
	}
	return My_Pass;
}
/*刷新列表数据*/
static void DisListText(void)
{
	unsigned char i=0;
	
	S_WifiPageBuffer->wifinum = 0;
	S_WifiPageBuffer->wifip = &(S_WifiPageBuffer->wifilist[S_WifiPageBuffer->pageindex*PageWifiNum]);
	
	for(i=0; i<PageWifiNum; i++)
	{
		ClearText(0x1E80+i*0x10, 30);
		if(strlen(S_WifiPageBuffer->wifip->ssid) > 0)
		{
			DisText(0x1E80+i*0x10, S_WifiPageBuffer->wifip->ssid, strlen(S_WifiPageBuffer->wifip->ssid));
			
			S_WifiPageBuffer->myico[i].X = 600;
			S_WifiPageBuffer->myico[i].Y = 142+i*40;
				
			if(S_WifiPageBuffer->wifip->indicator <= 10)
				S_WifiPageBuffer->myico[i].ICO_ID = 25;
			else if(S_WifiPageBuffer->wifip->indicator < 40)
				S_WifiPageBuffer->myico[i].ICO_ID = 26;
			else if(S_WifiPageBuffer->wifip->indicator < 70)
				S_WifiPageBuffer->myico[i].ICO_ID = 27;
			else
				S_WifiPageBuffer->myico[i].ICO_ID = 28;
			
			S_WifiPageBuffer->wifinum = i+1;
			S_WifiPageBuffer->wifip++;
		}
	}
	
	BasicUI(0x1F00 ,0x1907 , S_WifiPageBuffer->wifinum, S_WifiPageBuffer->myico , sizeof(Basic_ICO)*S_WifiPageBuffer->wifinum);
}


static void CheckIsNeedKey(void)
{
	static char *security = NULL;
	
	if(S_WifiPageBuffer)
	{
		if((S_WifiPageBuffer->selectindex <= 0)||(S_WifiPageBuffer->selectindex > PageWifiNum))
			return;
		
		S_WifiPageBuffer->wifip = &(S_WifiPageBuffer->wifilist[(S_WifiPageBuffer->pageindex)*PageWifiNum+S_WifiPageBuffer->selectindex-1]);
		
		security = strstr(S_WifiPageBuffer->wifip->auth, "OPEN");
		/*不需要密码*/
		if(security)
		{
			SendKeyCode(4);
			vTaskDelay(1000 / portTICK_RATE_MS);
			memcpy(S_WifiPageBuffer->wifip->key, "NONE", 4);
			if(My_Fail == ConnectWifi(S_WifiPageBuffer->wifip))
			{
				SendKeyCode(16);
				vTaskDelay(100 / portTICK_RATE_MS);
				SendKeyCode(2);
			}
			else
			{
				RestartWifi();
				vTaskDelay(2000/portTICK_RATE_MS);
				SendKeyCode(16);
				vTaskDelay(100 / portTICK_RATE_MS);
				SendKeyCode(1);
			}
		}
		/*已保存密码*/
		else if(My_Pass == ReadWifiData(S_WifiPageBuffer->wifip))
		{
			SendKeyCode(4);
			vTaskDelay(1000 / portTICK_RATE_MS);
			if(My_Fail == ConnectWifi(S_WifiPageBuffer->wifip))
			{
				SendKeyCode(16);
				vTaskDelay(100 / portTICK_RATE_MS);
				SendKeyCode(3);
				ClearWifiData(S_WifiPageBuffer->wifip);
			}
			else
			{
				RestartWifi();
				vTaskDelay(2000/portTICK_RATE_MS);
				SendKeyCode(16);
				vTaskDelay(100 / portTICK_RATE_MS);
				SendKeyCode(1);	
			}
		}
		/*输入密码*/
		else
			SendKeyCode(3);
	}
}


