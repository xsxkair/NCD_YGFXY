/***************************************************************************************************
*FileName:
*Description:
*Author: xsx_kair
*Data:
***************************************************************************************************/

/***************************************************************************************************/
/******************************************Header List********************************************/
/***************************************************************************************************/

#include	"LabServer_Fun.h"
#include	"Test_Fun.h"
#include	"Test_Task.h"
#include	"SystemSet_Dao.h"
#include	"Motor_Fun.h"

#include	"System_Data.h"
#include	"CRC16.h"
#include	"Define.h"
#include 	"tcpip.h"
#include	"MyMem.h"
#include	<string.h>
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
struct netconn *pxNetCon;
struct netbuf *pxRxBuffer;
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
static void ProcessCMD(unsigned char *buf, unsigned short len, struct netconn *pxNetCon);
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/****************************************File Start*************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

/***************************************************************************************************
*FunctionName: ProcessQuest
*Description: 处理客户端请求
*Input: parm -- 客户端
*Output: none
*Author: xsx
*Date: 2016年8月24日14:40:18
***************************************************************************************************/
void ProcessQuest(void * parm)
{
	unsigned char *pcRxString;
	unsigned short usLength;
	
	if(NULL == parm)
		return;
	
	pxNetCon = parm;
	/* We expect to immediately get data. */
	if(ERR_OK == netconn_recv( pxNetCon , &pxRxBuffer))
	{
		netbuf_data(pxRxBuffer, ( void * )&pcRxString, &usLength);

		netbuf_delete(pxRxBuffer);
		
		ProcessCMD(pcRxString ,usLength, pxNetCon);        
	}

}

static void ProcessCMD(unsigned char *buf, unsigned short len, struct netconn *pxNetCon)
{
	char *pxbuf1;
	char *pxbuf2;
	unsigned short temp = 0xffff;
	unsigned short i=0;
	
	pxbuf1 = MyMalloc(4096);
	pxbuf2 = MyMalloc(10);
	if(pxbuf1 && pxbuf2)
	{
		memset(pxbuf1, 0, 4096);
		
		if(!strncmp( (char *)buf, "Read Device Info", 16 ))
		{
			DeviceInfo * deviceinfo = MyMalloc(sizeof(DeviceInfo));
			if(deviceinfo)
			{
				//读取设备信息
				getDeviceInfo(deviceinfo);
				sprintf(pxbuf1, "{\"deviceid\":\"%s\",\"devicestatus\":%d}", deviceinfo->deviceid, GetTestStatusFlorLab());
			}
			MyFree(deviceinfo);
		}
		else if(!strncmp( (char *)buf, "Start Test", 10 ))
		{
			if(0 == GetTestStatusFlorLab())
			{
				StartTest(GetTestDataForLab());
				
				sprintf(pxbuf1, "OK");
			}
			else
				sprintf(pxbuf1, "Startted");
		}
		else if(!strncmp( (char *)buf, "Read Test Data", 14 ))
		{
			i = 0;
			
			sprintf(pxbuf1, "{\"data\":[");
			while(pdPASS == TakeTestPointData(&temp))
			{
				if(i == 0)
					sprintf(pxbuf2, "%d", temp);
				else
					sprintf(pxbuf2, ",%d", temp);
				strcat(pxbuf1, pxbuf2);
				i++;
			}
			sprintf(pxbuf2, "],\"status\":%d}", GetTestStatusFlorLab());
			strcat(pxbuf1, pxbuf2);
			
			if(0 == GetTestStatusFlorLab())
				MotorMoveTo(1, 2, MaxLocation, TRUE);
		}

		netconn_write( pxNetCon, pxbuf1, strlen(pxbuf1), NETCONN_COPY );
	}
	
	MyFree(pxbuf1);
	MyFree(pxbuf2);
}

/****************************************end of file************************************************/
