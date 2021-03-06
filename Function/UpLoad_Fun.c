/***************************************************************************************************
*FileName:
*Description:
*Author:
*Data:
***************************************************************************************************/

/***************************************************************************************************/
/******************************************头文件***************************************************/
/***************************************************************************************************/
#include	"UpLoad_Fun.h"
#include	"ServerFun.h"
#include	"RTC_Driver.h"
#include	"GPRS_Fun.h"
#include	"SystemSet_Dao.h"
#include	"System_Data.h"
#include	"RemoteSoftDao.h"
#include	"RemoteSoft_Data.h"
#include	"HttpBuf.h"
#include	"StringDefine.h"
#include	"Md5.h"
#include	"MyMem.h"
#include	"CRC16.h"
#include	"CheckSum.h"

#include	"FreeRTOS.h"
#include 	"task.h"
#include 	"queue.h"
#include	"semphr.h"

#include	<string.h>
#include	"stdio.h"
#include 	"stdlib.h"
/***************************************************************************************************/
/**************************************局部变量声明*************************************************/
/***************************************************************************************************/
static HttpBuffer * httpBuffer = NULL;
/***************************************************************************************************/
/**************************************局部函数声明*************************************************/
/***************************************************************************************************/
static void UpLoadDeviceInfo(HttpBuffer * httpBuffer);
static void UpLoadTestData(HttpBuffer * httpBuffer);
static void DownLoadFirmware(HttpBuffer * httpBuffer);
static void upLoadUserServer(void);
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************正文********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

void UpLoadFunction(void)
{
    unsigned short cnt = 0;
    
	while(1)
	{
		httpBuffer = MyMalloc(HttpBufferStructSize);
		if(httpBuffer)
		{
            #if (DEVICE_CON_TYPE == DEVICE_GPRS)
            if(My_Pass == GPRSInit())
            {
                if(cnt % 120 == 0)
                {    
                    set_device_state(TRUE);
                    cnt = 0;
                }
                    
            #elif (DEVICE_CON_TYPE == DEVICE_GPRS)
            {
            #endif //DEVICE_CON_TYPE
                if(deviceInfoIsNew())
                    UpLoadDeviceInfo(httpBuffer);
                vTaskDelay(100 / portTICK_RATE_MS);
                    
                UpLoadTestData(httpBuffer);	
                vTaskDelay(100 / portTICK_RATE_MS);
                    
                DownLoadFirmware(httpBuffer);
                vTaskDelay(100 / portTICK_RATE_MS);
			}
            
			upLoadUserServer();
		}
		
		MyFree(httpBuffer);
		
        cnt++;
        
		vTaskDelay(30000 / portTICK_RATE_MS);
	}
}

static void UpLoadDeviceInfo(HttpBuffer * httpBuffer)
{
	httpBuffer->device = (DeviceInfo *)httpBuffer->tempBuf;
	
	getDeviceInfo(httpBuffer->device);
	
	sprintf(httpBuffer->sendBuf, "POST %s HTTP/1.1\nHost: %d.%d.%d.%d:%d\nConnection: keep-alive\nContent-Length:[##]\nContent-Type:application/x-www-form-urlencoded;charset=GBK\nAccept-Language: zh-CN,zh;q=0.8\n\ndid=%s&dversion=%d&addr=%s&name=%s&age=%s&sex=%s&phone=%s&job=%s&dsc=%s&type=%s&lang=%s", 
		NcdServerUpDeviceUrlStr, GB_ServerIp_1, GB_ServerIp_2, GB_ServerIp_3, GB_ServerIp_4, GB_ServerPort, httpBuffer->device->deviceid,  
		GB_SoftVersion, httpBuffer->device->deviceunit, httpBuffer->device->deviceuser.user_name, 
		httpBuffer->device->deviceuser.user_age, httpBuffer->device->deviceuser.user_sex,	
		httpBuffer->device->deviceuser.user_phone, httpBuffer->device->deviceuser.user_job, 
		httpBuffer->device->deviceuser.user_desc, DeviceTypeString, DeviceLanguageString);
		
	httpBuffer->tempP = strstr(httpBuffer->sendBuf, "zh;q=0.8\n\n");
	httpBuffer->sendDataLen = strlen(httpBuffer->tempP)-10;	
	httpBuffer->tempP = strstr(httpBuffer->sendBuf, "[##]");
	sprintf(httpBuffer->tempBuf, "%04d", httpBuffer->sendDataLen);
	memcpy(httpBuffer->tempP, httpBuffer->tempBuf, 4);
	httpBuffer->sendDataLen = strlen(httpBuffer->sendBuf);
	httpBuffer->isPost = TRUE;
	
    #if (DEVICE_CON_TYPE == DEVICE_WIFI)
	if(My_Pass == CommunicateWithServerByWifi(httpBuffer))
    #elif (DEVICE_CON_TYPE == DEVICE_GPRS)
    if(My_Pass == CommunicateWithServerByGPRS(httpBuffer))
    #endif //DEVICE_CON_TYPE
	{
		httpBuffer->tempP2 = strtok(httpBuffer->tempP, "#");
		if(httpBuffer->tempP2 == NULL)
				return;
			
		//timedate
		httpBuffer->tempP2 = strtok(NULL, "#");
		if(httpBuffer->tempP2)
		{
			RTC_SetTimeData2(httpBuffer->tempP2+5);
		}
		else
			return;
			
		//version
		httpBuffer->tempP2 = strtok(NULL, "#");
		if(httpBuffer->tempP2)
		{
			httpBuffer->remoteSoftInfo = (RemoteSoftInfo *)httpBuffer->sendBuf;
			memset(httpBuffer->remoteSoftInfo, 0, RemoteSoftInfoStructSize);
			httpBuffer->remoteSoftInfo->RemoteFirmwareVersion = strtol(httpBuffer->tempP2+8, NULL, 10);
					
			//如果读取到的版本，大于当前版本，且大于当前保存的最新远程版本，则此次读取的是最新的
			if((httpBuffer->remoteSoftInfo->RemoteFirmwareVersion > GB_SoftVersion) &&
				(httpBuffer->remoteSoftInfo->RemoteFirmwareVersion > getGbRemoteFirmwareVersion()))
			{
				//md5
				httpBuffer->tempP2 = strtok(NULL, "#");
				if(httpBuffer->tempP2)
				{
					memcpy(httpBuffer->remoteSoftInfo->md5, httpBuffer->tempP2+4, 32);
					if(My_Pass == WriteRemoteSoftInfo(httpBuffer->remoteSoftInfo))
					{
						//md5保存成功后，才更新最新版本号，保存最新固件版本
						setGbRemoteFirmwareVersion(httpBuffer->remoteSoftInfo->RemoteFirmwareVersion);
						setGbRemoteFirmwareMd5(httpBuffer->remoteSoftInfo->md5);
								
						setIsSuccessDownloadFirmware(FALSE);
					}
				}
				else
					return;
			}	
		}
		else
			return;
	}
}

/***************************************************************************************************
*FunctionName: UpLoadTestData
*Description: 上传测试数据（数据和曲线）
*Input: 
*Output: 
*Return: 
*Author: xsx
*Date: 2017年2月20日16:43:44
***************************************************************************************************/
static void UpLoadTestData(HttpBuffer * httpBuffer)
{
	httpBuffer->tempInt1 = getTestDataTotalNum();
	httpBuffer->tempInt2 = getUpLoadIndex();
	
	//is have data not to update ?
	if(httpBuffer->tempInt1 > httpBuffer->tempInt2)
	{
		httpBuffer->page = MyMalloc(PageStructSize);
		if(httpBuffer->page)
		{
			//read datas from sd
			httpBuffer->pageRequest.startElementIndex = httpBuffer->tempInt2;
			httpBuffer->pageRequest.orderType = DESC;
			httpBuffer->pageRequest.pageSize = 5;

			memset(httpBuffer->page, 0, PageStructSize);
			
			if(My_Pass == ReadTestData(&httpBuffer->pageRequest, httpBuffer->page, httpBuffer->tempInt1))
			{
				httpBuffer->testData = httpBuffer->page->testData;                
                
				for(httpBuffer->upLoadIndex=0; httpBuffer->upLoadIndex< httpBuffer->page->ElementsSize; httpBuffer->upLoadIndex++)
				{
                    
					//如果crc校验正确，则开始上传
					if(httpBuffer->testData->crc == CalModbusCRC16Fun1(httpBuffer->testData, TestDataStructCrcSize))
					{ 
						//上传测试数据
						if(httpBuffer->testData->testResultDesc != ResultIsOK)
							sprintf(httpBuffer->tempBuf, "false");
						else
							sprintf(httpBuffer->tempBuf, "true");
						
						if(httpBuffer->testData->TestTime.month == 0 || httpBuffer->testData->TestTime.day == 0)
						{
							httpBuffer->testData->TestTime.year = 0;
							httpBuffer->testData->TestTime.month = 1;
							httpBuffer->testData->TestTime.day = 1;
							httpBuffer->testData->TestTime.hour = 0;
							httpBuffer->testData->TestTime.min = 0;
							httpBuffer->testData->TestTime.sec = 0;
						}
                        
                        if(httpBuffer->testData->testcnt > MAX_TEST_CNT)
                            httpBuffer->testData->testcnt = 0;
						
						//read device id
						readDeviceId(httpBuffer->tempBuf+10);
						readDeviceAddr(httpBuffer->tempBuf+100);
						
						sprintf(httpBuffer->sendBuf, "POST %s HTTP/1.1\nHost: %d.%d.%d.%d:%d\nConnection: keep-alive\nContent-Length:[##]\nContent-Type:application/x-www-form-urlencoded;charset=GBK\nAccept-Language: zh-CN,zh;q=0.8\n\ncardnum=%s&qrdata.cid=%s&device.did=%s&tester=%s&sampleid=%s&testtime=20%02d-%d-%d %d:%d:%d&overtime=%d&cline=%d&tline=%d&bline=%d&t_c_v=%.4f&t_tc_v=%.4f&testv=%.*f&serialnum=%s-%s&t_isok=%s&cparm=%d&t_cv=%.4f&c_cv=%.4f&testaddr=%s&errcode=%d&retestcnt=%d\0",
							NcdServerUpTestDataUrlStr, GB_ServerIp_1, GB_ServerIp_2, GB_ServerIp_3, GB_ServerIp_4, GB_ServerPort, 
							httpBuffer->testData->temperweima.piNum, httpBuffer->testData->temperweima.PiHao, 
							httpBuffer->tempBuf+10, httpBuffer->testData->user.user_name, 
							httpBuffer->testData->sampleid, httpBuffer->testData->TestTime.year, 
							httpBuffer->testData->TestTime.month, httpBuffer->testData->TestTime.day, 
							httpBuffer->testData->TestTime.hour, httpBuffer->testData->TestTime.min, 
							httpBuffer->testData->TestTime.sec, httpBuffer->testData->time, 
							httpBuffer->testData->testline.C_Point.x, httpBuffer->testData->testline.T_Point.x,
							httpBuffer->testData->testline.B_Point.x, httpBuffer->testData->testline.t_cValue, httpBuffer->testData->t_tcValue, 
							httpBuffer->testData->temperweima.itemConstData.pointNum, httpBuffer->testData->testline.BasicResult, 
							httpBuffer->testData->temperweima.PiHao, httpBuffer->testData->temperweima.piNum, httpBuffer->tempBuf,
							httpBuffer->testData->testline.CMdifyNum, httpBuffer->testData->t_cv, httpBuffer->testData->c_cv, httpBuffer->tempBuf+100,
                            httpBuffer->testData->testResultDesc, httpBuffer->testData->testcnt);

						for(httpBuffer->i=0; httpBuffer->i<100; httpBuffer->i++)
						{
							if(httpBuffer->i == 0)
								sprintf(httpBuffer->tempBuf, "&series=[%d,%d,%d", httpBuffer->testData->testline.TestPoint[httpBuffer->i*3],
									httpBuffer->testData->testline.TestPoint[httpBuffer->i*3+1], httpBuffer->testData->testline.TestPoint[httpBuffer->i*3+2]);
							else
								sprintf(httpBuffer->tempBuf, ",%d,%d,%d", httpBuffer->testData->testline.TestPoint[httpBuffer->i*3],
									httpBuffer->testData->testline.TestPoint[httpBuffer->i*3+1], httpBuffer->testData->testline.TestPoint[httpBuffer->i*3+2]);
							strcat(httpBuffer->sendBuf, httpBuffer->tempBuf);
						}
							
						strcat(httpBuffer->sendBuf, "]");
						
						httpBuffer->tempP = strstr(httpBuffer->sendBuf, "zh;q=0.8\n\n");
						httpBuffer->sendDataLen = strlen(httpBuffer->tempP)-10;	
						httpBuffer->tempP = strstr(httpBuffer->sendBuf, "[##]");
						sprintf(httpBuffer->tempBuf, "%04d", httpBuffer->sendDataLen);
						memcpy(httpBuffer->tempP, httpBuffer->tempBuf, 4);
						httpBuffer->sendDataLen = strlen(httpBuffer->sendBuf);
						httpBuffer->isPost = TRUE;
						
                        #if (DEVICE_CON_TYPE == DEVICE_WIFI)
                            if(My_Pass != CommunicateWithServerByWifi(httpBuffer))
                        #elif (DEVICE_CON_TYPE == DEVICE_GPRS)
                            if(My_Pass != CommunicateWithServerByGPRS(httpBuffer))
                        #endif //DEVICE_CON_TYPE
                            break;
					}

					httpBuffer->tempInt2++;
					httpBuffer->testData++;
				}
				
				httpBuffer->systemSetData = (SystemSetData *)httpBuffer->sendBuf;
				copyGBSystemSetData(httpBuffer->systemSetData);
				httpBuffer->systemSetData->upLoadIndex = httpBuffer->tempInt2;
				SaveSystemSetData(httpBuffer->systemSetData);
			}
		}
		
		MyFree(httpBuffer->page);
	}
}

static void DownLoadFirmware(HttpBuffer * httpBuffer)
{
	//检查是否有更新，且未成功下载，则需要下载
	if((getGbRemoteFirmwareVersion() > GB_SoftVersion) && (FALSE == getIsSuccessDownloadFirmware()))
	{
		memset(httpBuffer->tempBuf, 0, 100);
		readDeviceId(httpBuffer->tempBuf);
		
		sprintf(httpBuffer->sendBuf, "GET %s?softName=%s&lang=%s HTTP/1.1\nHost: %d.%d.%d.%d:%d\nConnection: keep-alive\n\n", 
			NcdServerDownSoftUrlStr, httpBuffer->tempBuf, DeviceLanguageString, GB_ServerIp_1, GB_ServerIp_2, GB_ServerIp_3, GB_ServerIp_4, GB_ServerPort);
		
		httpBuffer->sendDataLen = strlen(httpBuffer->sendBuf);
		httpBuffer->isPost = FALSE;
		
        #if (DEVICE_CON_TYPE == DEVICE_WIFI)
            CommunicateWithServerByWifi(httpBuffer);
        #elif (DEVICE_CON_TYPE == DEVICE_GPRS)
            CommunicateWithServerByGPRS(httpBuffer);
        #endif //DEVICE_CON_TYPE
	}
}

#if (UserProgramType == UserNormalProgram)
static void upLoadUserServer(void)
{
	httpBuffer->tempInt1 = getTestDataTotalNum();
	httpBuffer->tempInt2 = getUserUpLoadIndex();
	
	//is have data not to update ?
	if(httpBuffer->tempInt1 > httpBuffer->tempInt2)
	{
		httpBuffer->page = MyMalloc(PageStructSize);
		if(httpBuffer->page)
		{
			//read datas from sd
			httpBuffer->pageRequest.startElementIndex = httpBuffer->tempInt2;
			httpBuffer->pageRequest.orderType = DESC;
			httpBuffer->pageRequest.pageSize = 3;

			memset(httpBuffer->page, 0, PageStructSize);
			
			if(My_Pass == ReadTestData(&httpBuffer->pageRequest, httpBuffer->page, httpBuffer->tempInt1))
			{
				httpBuffer->testData = httpBuffer->page->testData;
				for(httpBuffer->upLoadIndex=0; httpBuffer->upLoadIndex< httpBuffer->page->ElementsSize; httpBuffer->upLoadIndex++)
				{
					//如果crc校验正确，则开始上传
					if(httpBuffer->testData->crc == CalModbusCRC16Fun1(httpBuffer->testData, TestDataStructCrcSize))
					{
						//上传测试数据
						if(httpBuffer->testData->TestTime.month == 0 || httpBuffer->testData->TestTime.day == 0)
						{
							httpBuffer->testData->TestTime.year = 0;
							httpBuffer->testData->TestTime.month = 1;
							httpBuffer->testData->TestTime.day = 1;
							httpBuffer->testData->TestTime.hour = 0;
							httpBuffer->testData->TestTime.min = 0;
							httpBuffer->testData->TestTime.sec = 0;
						}
						
						//read device id
						readDeviceId(httpBuffer->tempBuf);
						
						//AA | testtime | sampleid | testtype | pihao | pinum | deviceid | tester | item | danwei | normal 
						sprintf(httpBuffer->sendBuf, "AA|20%02d-%d-%d %d:%d:%d|%s|%s|%s|%s|%s|%s|%s|%s|%s",  
							httpBuffer->testData->TestTime.year,  httpBuffer->testData->TestTime.month, httpBuffer->testData->TestTime.day, 
							httpBuffer->testData->TestTime.hour, httpBuffer->testData->TestTime.min, httpBuffer->testData->TestTime.sec,
							httpBuffer->testData->sampleid, ChangguiStr, httpBuffer->testData->temperweima.PiHao,  httpBuffer->testData->temperweima.piNum,
							httpBuffer->tempBuf, httpBuffer->testData->user.user_name, httpBuffer->testData->temperweima.itemConstData.itemName,
							httpBuffer->testData->temperweima.itemConstData.itemMeasure, httpBuffer->testData->temperweima.itemConstData.normalResult);

						// | value | error | BB
						if(httpBuffer->testData->testResultDesc != ResultIsOK)
							sprintf(httpBuffer->tempBuf, "||Y|BB");
						else if(httpBuffer->testData->testline.BasicResult <= httpBuffer->testData->temperweima.itemConstData.lowstResult)
							sprintf(httpBuffer->tempBuf, "|<%.*f|N|BB", httpBuffer->testData->temperweima.itemConstData.pointNum, 
								httpBuffer->testData->temperweima.itemConstData.lowstResult);
						else
							sprintf(httpBuffer->tempBuf, "|%.*f|N|BB", httpBuffer->testData->temperweima.itemConstData.pointNum, 
								httpBuffer->testData->testline.BasicResult);
						strcat(httpBuffer->sendBuf, httpBuffer->tempBuf);
						strcat(httpBuffer->sendBuf, "\r\n");
						
						httpBuffer->sendDataLen = strlen(httpBuffer->sendBuf);	
						
						getGBServerData(&httpBuffer->serverSet);
						if(My_Pass == CommunicateWithServerByUSB(httpBuffer) || My_Pass == CommunicateWithServerByLineNet(httpBuffer))
						{
							if(strstr(httpBuffer->recvBuf, httpBuffer->testData->temperweima.PiHao) && strstr(httpBuffer->recvBuf, httpBuffer->testData->temperweima.piNum))
								;
							else
								break;
						}
						else
							break;
					}

					httpBuffer->tempInt2++;
					httpBuffer->testData++;
				}
				
				httpBuffer->systemSetData = (SystemSetData *)httpBuffer->sendBuf;
				copyGBSystemSetData(httpBuffer->systemSetData);
				httpBuffer->systemSetData->userUpLoadIndex = httpBuffer->tempInt2;
				SaveSystemSetData(httpBuffer->systemSetData);
			}
		}
		
		MyFree(httpBuffer->page);
	}
}
#elif (UserProgramType == UserXGProgram)
static void upLoadUserServer(void)
{
	httpBuffer->tempInt1 = getTestDataTotalNum();
	httpBuffer->tempInt2 = getUserUpLoadIndex();
	
	//is have data not to update ?
	if(httpBuffer->tempInt1 > httpBuffer->tempInt2)
	{
		httpBuffer->page = MyMalloc(PageStructSize);
		if(httpBuffer->page)
		{
			//read datas from sd
			httpBuffer->pageRequest.startElementIndex = httpBuffer->tempInt2;
			httpBuffer->pageRequest.orderType = DESC;
			httpBuffer->pageRequest.pageSize = 3;

			memset(httpBuffer->page, 0, PageStructSize);
			
			if(My_Pass == ReadTestData(&httpBuffer->pageRequest, httpBuffer->page, httpBuffer->tempInt1))
			{
				httpBuffer->testData = httpBuffer->page->testData;
				for(httpBuffer->upLoadIndex=0; httpBuffer->upLoadIndex< httpBuffer->page->ElementsSize; httpBuffer->upLoadIndex++)
				{
					//如果crc校验正确，则开始上传
					if(httpBuffer->testData->crc == CalModbusCRC16Fun1(httpBuffer->testData, TestDataStructCrcSize))
					{
						//上传测试数据
						if(httpBuffer->testData->TestTime.month == 0 || httpBuffer->testData->TestTime.day == 0)
						{
							httpBuffer->testData->TestTime.year = 0;
							httpBuffer->testData->TestTime.month = 1;
							httpBuffer->testData->TestTime.day = 1;
							httpBuffer->testData->TestTime.hour = 0;
							httpBuffer->testData->TestTime.min = 0;
							httpBuffer->testData->TestTime.sec = 0;
						}
						
						//read device id
						readDeviceId(httpBuffer->tempBuf);
						
						//STX
						httpBuffer->sendBuf[0] = 0x02;
						//AA | testtime | sampleid | testtype | pihao | pinum | deviceid | tester | item | danwei | normal 
						sprintf(httpBuffer->sendBuf+1, "H|\\^&|||荧光免疫定量分析仪^%s^%s^NCD-A01^%s-%s^1.0|||||||P||20%02d%02d%02d%02d%02d%02d\rP|1||||^^\rO|1|%s|||||||||||||Blood\rR|1|^^^%s",  
							GB_SoftVersionStr, httpBuffer->tempBuf, httpBuffer->testData->temperweima.PiHao,  httpBuffer->testData->temperweima.piNum,
							httpBuffer->testData->TestTime.year,  httpBuffer->testData->TestTime.month, httpBuffer->testData->TestTime.day, 
							httpBuffer->testData->TestTime.hour, httpBuffer->testData->TestTime.min, httpBuffer->testData->TestTime.sec,
							httpBuffer->testData->sampleid, httpBuffer->testData->temperweima.itemConstData.itemName);

						// | value | error | BB
						if(httpBuffer->testData->testResultDesc != ResultIsOK)
							sprintf(httpBuffer->tempBuf, "|error|");
						else if(httpBuffer->testData->testline.BasicResult <= httpBuffer->testData->temperweima.itemConstData.lowstResult)
							sprintf(httpBuffer->tempBuf, "|<%.*f|", httpBuffer->testData->temperweima.itemConstData.pointNum, httpBuffer->testData->temperweima.itemConstData.lowstResult);
						else
							sprintf(httpBuffer->tempBuf, "|%.*f|", httpBuffer->testData->temperweima.itemConstData.pointNum, httpBuffer->testData->testline.BasicResult);
						strcat(httpBuffer->sendBuf, httpBuffer->tempBuf);
						
						sprintf(httpBuffer->tempBuf, "%s|%s^^^|L||||1\rL|1\r", httpBuffer->testData->temperweima.itemConstData.itemMeasure, httpBuffer->testData->temperweima.itemConstData.normalResult);
						strcat(httpBuffer->sendBuf, httpBuffer->tempBuf);
						httpBuffer->sendDataLen = strlen(httpBuffer->sendBuf);
						httpBuffer->sendBuf[httpBuffer->sendDataLen] = 0x03;
						httpBuffer->sendDataLen++;
						
						httpBuffer->sendBuf[httpBuffer->sendDataLen] = CheckSumFun(httpBuffer->sendBuf+1, httpBuffer->sendDataLen-1, NULL);
						httpBuffer->sendDataLen++;
						httpBuffer->sendBuf[httpBuffer->sendDataLen] = 0x0d;
						httpBuffer->sendDataLen++;
						httpBuffer->sendBuf[httpBuffer->sendDataLen] = 0x0a;
						httpBuffer->sendDataLen++;
						httpBuffer->sendBuf[httpBuffer->sendDataLen] = 0x04;
						httpBuffer->sendDataLen++;
						
						getGBServerData(&httpBuffer->serverSet);
						if(My_Pass == CommunicateWithLisByLineNet(httpBuffer))
							;
						else
							break;
					}

					httpBuffer->tempInt2++;
					httpBuffer->testData++;
				}
				
				httpBuffer->systemSetData = (SystemSetData *)httpBuffer->sendBuf;
				copyGBSystemSetData(httpBuffer->systemSetData);
				httpBuffer->systemSetData->userUpLoadIndex = httpBuffer->tempInt2;
				SaveSystemSetData(httpBuffer->systemSetData);
			}
		}
		
		MyFree(httpBuffer->page);
	}
}
#endif

