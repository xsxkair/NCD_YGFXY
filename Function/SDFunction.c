/***************************************************************************************************
*FileName:SDFunction
*Description:一些SD卡的操作函数
*Author:xsx
*Data:2016年4月30日16:06:36
***************************************************************************************************/


/***************************************************************************************************/
/******************************************头文件***************************************************/
/***************************************************************************************************/
#include	"SDFunction.h"
#include	"System_Data.h"

#include	"CRC16.h"
#include	"MyMem.h"
#include	"MyTools.h"

#include 	"FreeRTOS.h"
#include 	"task.h"
#include 	"queue.h"
#include	"semphr.h"

#include	<string.h>
#include	"stdio.h"
#include 	"stdlib.h"





/***************************************************************************************************/
/**************************************局部变量声明*************************************************/
/***************************************************************************************************/




/***************************************************************************************************/
/**************************************局部函数声明*************************************************/
/***************************************************************************************************/





/***************************************************************************************************/
/***************************************************************************************************/
/***************************************正文********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

/***************************************************************************************************
*FunctionName：CheckSDFunction
*Description：检测SD卡是否正常
*Input：None
*Output：None
*Author：xsx
*Data：2016年4月30日16:38:48
***************************************************************************************************/
MyState_TypeDef CheckSDFunction(void)
{
	FatfsFileInfo_Def * myfile = NULL;
	unsigned char *buf = NULL;
	MyState_TypeDef statues = My_Fail;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	buf = MyMalloc(100);
	
	if(myfile && buf)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));
		memset(buf, 0, 100);

		myfile->res = f_open(&(myfile->file), "0:/SDCheck.ncd", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
			
		if(FR_OK == myfile->res)
		{
			myfile->res = f_write(&(myfile->file), "0123456789", 10, &(myfile->bw));
			if(FR_OK == myfile->res)
			{
				f_lseek(&(myfile->file), 0);
				myfile->res = f_read(&(myfile->file), buf, 10, &(myfile->br));
				if((FR_OK == myfile->res) && (0 == memcmp(buf, "0123456789", 10)))
					statues = My_Pass;
			}

			f_close(&(myfile->file));
		}
	}
	
	MyFree(buf);
	MyFree(myfile);
	
	return statues;
}



/***************************************************************************************************/
/***************************************************************************************************/
/***************************************读写操作人**************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
MyState_TypeDef SaveUserData(User_Type * user)
{
	FatfsFileInfo_Def * myfile = NULL;
	MyState_TypeDef statues = My_Fail;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	
	if(myfile)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));

		myfile->res = f_open(&(myfile->file), "0:/Testers.ncd", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
			
		if(FR_OK == myfile->res)
		{
			f_lseek(&(myfile->file), 0);
			
			myfile->res = f_write(&(myfile->file), user, sizeof(User_Type)*MaxUserNum, &(myfile->bw));
				
			if((FR_OK == myfile->res)&&(myfile->bw == sizeof(User_Type)*MaxUserNum))
				statues = My_Pass;
				
			f_close(&(myfile->file));
		}
	}
	
	MyFree(myfile);
	
	return statues;
}

MyState_TypeDef ReadUserData(User_Type * user)
{
	FatfsFileInfo_Def * myfile = NULL;
	MyState_TypeDef statues = My_Fail;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));

	if(myfile && user)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));

		myfile->res = f_open(&(myfile->file), "0:/Testers.ncd", FA_READ);
		
		if(FR_OK == myfile->res)
		{
			f_lseek(&(myfile->file), 0);
					
			myfile->res = f_read(&(myfile->file), user, sizeof(User_Type)*MaxUserNum, &(myfile->br));

			if((FR_OK == myfile->res)&&(myfile->br == sizeof(User_Type)*MaxUserNum))
				statues = My_Pass;
			
			f_close(&(myfile->file));
		}
	}	
	MyFree(myfile);
	
	return statues;
}
/*
MyState_TypeDef ReadIndexPlus(unsigned char num)
{
	FatfsFileInfo_Def * myfile = NULL;
	TestDataSaveHead * myTestDataSaveHead;
	MyState_TypeDef statues = My_Fail;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	myTestDataSaveHead = MyMalloc(sizeof(TestDataSaveHead));
	
	if(myfile && myTestDataSaveHead)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));
		memset(myTestDataSaveHead, 0, sizeof(TestDataSaveHead));
		
		myfile->res = f_open(&(myfile->file), "0:/TD.NCD", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
			
		if(FR_OK == myfile->res)
		{
			//读取数据头
			f_lseek(&(myfile->file), 0);
			f_read(&(myfile->file), myTestDataSaveHead, sizeof(TestDataSaveHead), &(myfile->br));
			
			//如果数据错误，则不更新此次上传索引
			if(myTestDataSaveHead->crc != CalModbusCRC16Fun1(myTestDataSaveHead, sizeof(TestDataSaveHead)-2))
			{
				myTestDataSaveHead->datanum = 0;
				myTestDataSaveHead->readindex = 0;
			}
			else
				myTestDataSaveHead->readindex += num;
			
			myTestDataSaveHead->crc = CalModbusCRC16Fun1(myTestDataSaveHead, sizeof(TestDataSaveHead)-2);
			
			//写数据头
			myfile->res = f_lseek(&(myfile->file), 0);
			myfile->res = f_write(&(myfile->file), myTestDataSaveHead, sizeof(TestDataSaveHead), &(myfile->bw));
			if(FR_OK == myfile->res)
				statues = My_Pass;
			
			f_close(&(myfile->file));
		}
	}
	
	MyFree(myfile);
	MyFree(myTestDataSaveHead);
	
	return statues;
}
MyState_TypeDef ReadTestDataHead(TestDataSaveHead * head)
{
	FatfsFileInfo_Def * myfile = NULL;
	MyState_TypeDef statues = My_Fail;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	
	if(myfile)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));

		myfile->res = f_open(&(myfile->file), "0:/TD.NCD", FA_READ);
			
		if(FR_OK == myfile->res)
		{	
			myfile->res = f_lseek(&(myfile->file), 0);
			myfile->res = f_read(&(myfile->file), head, sizeof(TestDataSaveHead), &(myfile->br));
			
			if(FR_OK == myfile->res)
				statues = My_Pass;
			
			f_close(&(myfile->file));
		}
	}
	
	MyFree(myfile);
	
	return statues;
}
*/
/***************************************************************************************************/
/***************************************************************************************************/
/*************************************IP设置********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
MyState_TypeDef SaveNetData(NetData * netdata)
{
	FatfsFileInfo_Def * myfile = NULL;
	MyState_TypeDef statues = My_Fail;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));

	if(myfile)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));

		myfile->res = f_open(&(myfile->file), "0:/NetSet.ncd", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
			
		if(FR_OK == myfile->res)
		{
			f_lseek(&(myfile->file), 0);
				
			netdata->crc = CalModbusCRC16Fun1(netdata, sizeof(NetData)-2);
				
			myfile->res = f_write(&(myfile->file), netdata, sizeof(NetData), &(myfile->bw));
				
			if((FR_OK == myfile->res)&&(myfile->bw == sizeof(NetData)))
				statues = My_Pass;

			myfile->res = f_close(&(myfile->file));
		}
	}
	
	MyFree(myfile);
	
	return statues;
}

MyState_TypeDef ReadNetData(NetData * netdata)
{
	FatfsFileInfo_Def * myfile = NULL;
	MyState_TypeDef statues = My_Fail;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	
	if(myfile)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));

		myfile->res = f_open(&(myfile->file), "0:/NetSet.ncd", FA_READ);
		
		if(FR_OK == myfile->res)
		{
			f_lseek(&(myfile->file), 0);
					
			myfile->res = f_read(&(myfile->file), netdata, sizeof(NetData), &(myfile->br));
			if((FR_OK == myfile->res)&&(myfile->br == sizeof(NetData))&&(netdata->crc == CalModbusCRC16Fun1(netdata, sizeof(NetData)-2)))
			{
				statues = My_Pass;
			}

			myfile->res = f_close(&(myfile->file));
		}
	}
	MyFree(myfile);
	
	return statues;
}
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************wifi密码保存************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
MyState_TypeDef SaveWifiData(WIFI_Def * wifi)
{
	FatfsFileInfo_Def * myfile = NULL;
	WIFI_Def * tempwifi = NULL;
	MyState_TypeDef statues = My_Fail;
	unsigned char i=0;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	tempwifi = MyMalloc(sizeof(WIFI_Def));
	if(myfile && tempwifi)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));
		memset(tempwifi, 0, sizeof(WIFI_Def));

		myfile->res = f_open(&(myfile->file), "0:/WifiSet.ncd", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
			
		if(FR_OK == myfile->res)
		{
			myfile->size = f_size(&(myfile->file));

			for(i=0; i<255; i++)
			{
				f_lseek(&(myfile->file), i*sizeof(WIFI_Def));
				myfile->res = f_read(&(myfile->file), tempwifi, sizeof(WIFI_Def), &(myfile->br));
				if((FR_OK == myfile->res)&&(myfile->br == sizeof(WIFI_Def))&&(tempwifi->crc == CalModbusCRC16Fun1(tempwifi, sizeof(WIFI_Def)-2)))
				{
					if(pdPASS == CheckStrIsSame(wifi->ssid, tempwifi->ssid, MaxSSIDLen))
					{
						break;
					}
				}
				else
					break;
			}
			if(i<255)
			{
				f_lseek(&(myfile->file), i*sizeof(WIFI_Def));
				wifi->crc = CalModbusCRC16Fun1(wifi, sizeof(WIFI_Def)-2);
				myfile->res = f_write(&(myfile->file), wifi, sizeof(WIFI_Def), &(myfile->bw));
				if((FR_OK == myfile->res)&&(myfile->bw == sizeof(WIFI_Def)))
					statues = My_Pass;
			}

			f_close(&(myfile->file));
		}
	}
	MyFree(tempwifi);
	MyFree(myfile);
	
	return statues;
}

MyState_TypeDef ReadWifiData(WIFI_Def * wifi)
{
	FatfsFileInfo_Def * myfile = NULL;
	MyState_TypeDef statues = My_Fail;
	unsigned char i = 0;
	WIFI_Def * tempwifi = NULL;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	tempwifi = MyMalloc(sizeof(WIFI_Def));
	
	if(myfile && tempwifi)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));
		memset(tempwifi, 0, sizeof(WIFI_Def));
		
		myfile->res = f_open(&(myfile->file), "0:/WifiSet.ncd", FA_READ);
		
		if(FR_OK == myfile->res)
		{
			myfile->size = f_size(&(myfile->file));
			
			for(i=0; i<(myfile->size / sizeof(WIFI_Def)); i++)
			{
				f_lseek(&(myfile->file), i*sizeof(WIFI_Def));
				
				memset(tempwifi, 0, sizeof(WIFI_Def));
				myfile->res = f_read(&(myfile->file), tempwifi, sizeof(WIFI_Def), &(myfile->br));
				if((FR_OK == myfile->res)&&(myfile->br == sizeof(WIFI_Def)))
				{
					if((tempwifi->crc == CalModbusCRC16Fun1(tempwifi, sizeof(WIFI_Def)-2))&&(pdPASS == CheckStrIsSame(wifi->ssid, tempwifi->ssid, MaxSSIDLen)))
					{
						memcpy(wifi->key, tempwifi->key, MaxKEYLen);
						statues = My_Pass;
						break;
					}
				}
			}

			f_close(&(myfile->file));
		}
	}
	
	MyFree(tempwifi);
	MyFree(myfile);
	
	return statues;
}

MyState_TypeDef ClearWifiData(WIFI_Def * wifi)
{
	FatfsFileInfo_Def * myfile = NULL;
	WIFI_Def * tempwifi = NULL;
	MyState_TypeDef statues = My_Fail;
	unsigned short i=0;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	tempwifi = MyMalloc(sizeof(WIFI_Def));
	
	if(myfile && tempwifi)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));
		memset(tempwifi, 0, sizeof(WIFI_Def));

		myfile->res = f_open(&(myfile->file), "0:/WifiSet.ncd", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
			
		if(FR_OK == myfile->res)
		{
			myfile->size = f_size(&(myfile->file));
			
			for(i=0; i<(myfile->size / sizeof(WIFI_Def)); i++)
			{
				f_lseek(&(myfile->file), i*sizeof(WIFI_Def));
				
				memset(tempwifi, 0, sizeof(WIFI_Def));
				myfile->res = f_read(&(myfile->file), tempwifi, sizeof(WIFI_Def), &(myfile->br));
				if((FR_OK == myfile->res)&&(myfile->br == sizeof(WIFI_Def)))
				{
					if((tempwifi->crc == CalModbusCRC16Fun1(tempwifi, sizeof(WIFI_Def)-2))&&(pdPASS == CheckStrIsSame(wifi->ssid, tempwifi->ssid, MaxSSIDLen)))
					{
						tempwifi->crc = 0;
						myfile->res = f_write(&(myfile->file), tempwifi, sizeof(WIFI_Def), &(myfile->bw));
						if((FR_OK == myfile->res)&&(myfile->bw == sizeof(WIFI_Def)))
							statues = My_Pass;
						break;
					}
				}
			}

			f_close(&(myfile->file));
		}
	}
	MyFree(tempwifi);
	MyFree(myfile);
	
	return statues;
}

/*********************************************************************************************/
/*********************************************************************************************/
/************************************读取校准参数*********************************************/
/*********************************************************************************************/
/*********************************************************************************************/
MyState_TypeDef SaveAdjustData(AdjustData *adjustdata)
{
	FatfsFileInfo_Def * myfile = NULL;
	AdjustData * tempadjust = NULL;
	MyState_TypeDef statues = My_Fail;
	unsigned char i=0;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	tempadjust = MyMalloc(sizeof(AdjustData));
	
	if(myfile && adjustdata && tempadjust)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));
		memset(tempadjust, 0, sizeof(AdjustData));

		myfile->res = f_open(&(myfile->file), "0:/AdjustD.ncd", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
			
		if(FR_OK == myfile->res)
		{
			myfile->size = f_size(&(myfile->file));
			f_lseek(&(myfile->file), 0);
			
			for(i=0; i<255; i++)
			{
				myfile->res = f_read(&(myfile->file), tempadjust, sizeof(AdjustData), &(myfile->br));
				if((FR_OK == myfile->res)&&(myfile->br == sizeof(AdjustData))&&(tempadjust->crc == CalModbusCRC16Fun1(tempadjust, sizeof(AdjustData)-2)))
				{
					if(pdPASS == CheckStrIsSame(tempadjust->ItemName, adjustdata->ItemName, ItemNameLen))
					{
						break;
					}
				}
				else
					break;
			}
			if(i<255)
			{
				f_lseek(&(myfile->file), i*sizeof(WIFI_Def));
				adjustdata->crc = CalModbusCRC16Fun1(adjustdata, sizeof(AdjustData)-2);
				myfile->res = f_write(&(myfile->file), adjustdata, sizeof(AdjustData), &(myfile->bw));
				if((FR_OK == myfile->res)&&(myfile->bw == sizeof(AdjustData)))
					statues = My_Pass;
			}

			f_close(&(myfile->file));
		}
	}
	MyFree(tempadjust);
	MyFree(myfile);
	
	return statues;
}

MyState_TypeDef ReadAdjustData(AdjustData *adjustdata)
{
	FatfsFileInfo_Def * myfile = NULL;
	AdjustData * tempadjust = NULL;
	MyState_TypeDef statues = My_Fail;
	unsigned char i=0;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	tempadjust = MyMalloc(sizeof(AdjustData));
	
	if(myfile && adjustdata && tempadjust)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));
		memset(tempadjust, 0, sizeof(AdjustData));

		myfile->res = f_open(&(myfile->file), "0:/AdjustD.ncd", FA_READ);
			
		if(FR_OK == myfile->res)
		{
			myfile->size = f_size(&(myfile->file));
			f_lseek(&(myfile->file), 0);
			
			for(i=0; i<255; i++)
			{
				myfile->res = f_read(&(myfile->file), tempadjust, sizeof(AdjustData), &(myfile->br));
				if((FR_OK == myfile->res)&&(myfile->br == sizeof(AdjustData))&&(tempadjust->crc == CalModbusCRC16Fun1(tempadjust, sizeof(AdjustData)-2)))
				{
					if(pdPASS == CheckStrIsSame(tempadjust->ItemName, adjustdata->ItemName, ItemNameLen))
					{
						adjustdata->parm = tempadjust->parm;
						adjustdata->crc = CalModbusCRC16Fun1(adjustdata, sizeof(AdjustData)-2);
						statues = My_Pass;
						break;
					}
				}
				else
					break;
			}
			f_close(&(myfile->file));
		}
	}
	MyFree(tempadjust);
	MyFree(myfile);
	
	return statues;
}

/*********************************************************************************************/
/*********************************************************************************************/
/************************************写入老化数据*********************************************/
/*********************************************************************************************/
/*********************************************************************************************/

MyState_TypeDef SaveReTestData(ReTestData *retestdata, unsigned char type)
{
	FatfsFileInfo_Def * myfile = NULL;
	char *buf;
	MyState_TypeDef statues = My_Fail;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));
	buf = MyMalloc(1024);
	
	if(myfile && retestdata && buf)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));
		
		myfile->res = f_open(&(myfile->file), "0:/laohua.csv", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
			
		if(FR_OK == myfile->res)
		{
			myfile->size = f_size(&(myfile->file));
			f_lseek(&(myfile->file), myfile->size);
			
			if(myfile->size == 0)
			{
				memset(buf, 0, 1024);
				sprintf(buf, "测试次数,测试时间,测试时长(秒),结果描述,[DA-AD],[DA-AD],[DA-AD],LED状态,环境温度,机壳内温度,检测卡温度,[T值-T位置],[C值-C位置],[B值-B位置],峰高比,原始结果,校准结果,当前音频起始时间,当前音频结束时间,当前音频时长,音频总时长,音频播放次数\r");
				myfile->res = f_write(&(myfile->file), buf, strlen(buf), &(myfile->bw));
				if(FR_OK != myfile->res)
					goto END;
			}
			
			//保存测试数据
			if(type == 0)
			{
				memset(buf, 0, 1024);
				sprintf(buf, "%d/%d,%d-%d-%d %d:%d:%d,%d,%s,[100-%.3f],[200-%.3f],[300-%.3f],%d,%.1f,%.1f,%.1f,[%d-%d],[%d-%d],[%d-%d],%.3f,%.3f,%.3f\r", retestdata->retestedcount, retestdata->retestcount, retestdata->testdata.TestTime.year
					, retestdata->testdata.TestTime.month, retestdata->testdata.TestTime.day, retestdata->testdata.TestTime.hour, retestdata->testdata.TestTime.min, retestdata->testdata.TestTime.sec
					, timer_Count(&(retestdata->oneretesttimer)), retestdata->result, retestdata->advalue1, retestdata->advalue2, retestdata->advalue3, retestdata->ledstatus, retestdata->testdata.TestTemp.E_Temperature, retestdata->testdata.TestTemp.I_Temperature
					, retestdata->testdata.TestTemp.O_Temperature, retestdata->testdata.testline.T_Point[0], retestdata->testdata.testline.T_Point[1], retestdata->testdata.testline.C_Point[0]
					, retestdata->testdata.testline.C_Point[1], retestdata->testdata.testline.B_Point[0], retestdata->testdata.testline.B_Point[1], retestdata->testdata.testline.BasicBili
					, retestdata->testdata.testline.BasicResult, retestdata->testdata.testline.AdjustResult);
				myfile->res = f_write(&(myfile->file), buf, strlen(buf), &(myfile->bw));
				
				if(FR_OK != myfile->res)
					goto END;
				
				statues = My_Pass;
			}
			//保存音频测试数据
			else
			{
				memset(buf, 0, 1024);
				sprintf(buf, ",,,,,,,,,,,,,,,,,%d-%d-%d %d:%d:%d,%d-%d-%d %d:%d:%d,%d,%d,%d\r", retestdata->startplayTime.year, retestdata->startplayTime.month, retestdata->startplayTime.day
					,retestdata->startplayTime.hour, retestdata->startplayTime.min, retestdata->startplayTime.sec, retestdata->endplayTime.year, retestdata->endplayTime.month
					, retestdata->endplayTime.day, retestdata->endplayTime.hour, retestdata->endplayTime.min, retestdata->endplayTime.sec, timer_Count(&(retestdata->oneplaytimer))
					, timer_Count(&(retestdata->playtimer)), retestdata->playcount);
				myfile->res = f_write(&(myfile->file), buf, strlen(buf), &(myfile->bw));
				
				if(FR_OK != myfile->res)
					goto END;
				
				statues = My_Pass;
			}
			
			END:
				f_close(&(myfile->file));
		}
	}
	
	MyFree(myfile);
	MyFree(buf);
	
	return statues;
}

