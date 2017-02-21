/***************************************************************************************************
*FileName : TestLinks_Data
*Description: ������������
*Author:xsx
*Data: 2016��5��14��17:14:02
***************************************************************************************************/

/***************************************************************************************************/
/******************************************ͷ�ļ�***************************************************/
/***************************************************************************************************/
#include	"MyTest_Data.h"
#include	"SystemSet_Data.h"

#include	"MyMem.h"
#include	"string.h"
#include 	"FreeRTOS.h"
#include 	"task.h"
/***************************************************************************************************/
/**************************************����*************************************************/
/***************************************************************************************************/
//�������ݻ�����
static TestBuffer GB_TestBuffer = {
	.NormalTestDataBuffer = NULL,
	.PaiduiTestDataBuffer = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	.CurrentTestDataBuffer = NULL,
};
/***************************************************************************************************/
/**************************************�ڲ�����*************************************************/
/***************************************************************************************************/

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************����********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

/***************************************************************************************************
*FunctionName: IsPaiDuiTestting
*Description: ����Ƿ�ǰ�ڽ�����������
*Input: None
*Output: None
*Return: 	true -- �п����ŶӲ�����
*			false -- û�н�����������
*Author: xsx
*Date: 2016��12��2��15:10:07
***************************************************************************************************/
bool IsPaiDuiTestting(void)
{
	unsigned char i=0;

	for(i=0; i<PaiDuiWeiNum; i++)
	{
		if(GB_TestBuffer.PaiduiTestDataBuffer[i] != NULL)
			return true;
	}
	
	return false;
}

CreateTestErrorType CreateANewTest(TestType testtype)
{
	unsigned char i=0;
	
	if(GB_TestBuffer.CurrentTestDataBuffer != NULL)
		return Error_PaiduiTesting;
	
	//����ǳ������
	if(testtype == NormalTestType)
	{
		//����������Թ�����ʹ���У����ֹ�������
		if(true == IsPaiDuiTestting())
			return Error_StopNormalTest;
		
		//��������ڴ�
		GB_TestBuffer.NormalTestDataBuffer = (ItemData *)MyMalloc(sizeof(ItemData));
		
		//�ڴ�����ʧ��
		if(GB_TestBuffer.NormalTestDataBuffer == NULL)
			return Error_Mem;
		else
		{
			GB_TestBuffer.CurrentTestDataBuffer = GB_TestBuffer.NormalTestDataBuffer;
			memset(GB_TestBuffer.CurrentTestDataBuffer, 0, sizeof(ItemData));
			GB_TestBuffer.NormalTestDataBuffer->testlocation = 0;
			
			//��ϵͳ���������л�ȡ����ʱled������ֵ
			GB_TestBuffer.NormalTestDataBuffer->ledLight = getTestLedLightIntensity(getGBSystemSetData());
			
			return Error_OK;
		}
	}
	else
	{
		if(60 > GetMinWaitTime())
			return Error_PaiDuiBusy;
		
		for(i=0; i<PaiDuiWeiNum; i++)
		{
			if(GB_TestBuffer.PaiduiTestDataBuffer[i] == NULL)
			{
				//��������ڴ�
				GB_TestBuffer.PaiduiTestDataBuffer[i] = (ItemData *)MyMalloc(sizeof(ItemData));
				
				//�ڴ�����ʧ��
				if(GB_TestBuffer.PaiduiTestDataBuffer[i] == NULL)
					return Error_Mem;
				else
				{
					GB_TestBuffer.CurrentTestDataBuffer = GB_TestBuffer.PaiduiTestDataBuffer[i];
					memset(GB_TestBuffer.CurrentTestDataBuffer, 0, sizeof(ItemData));
					GB_TestBuffer.CurrentTestDataBuffer->testlocation = i + 1;
					
					//�����Ŷӹ��ò����˵���ǰ���������У�����ǵ�һ�δ����Ŷӣ�����ѡ���������Ḳ�Ǵ˴β���
					memcpy(&(GB_TestBuffer.CurrentTestDataBuffer->testdata.user), &(GB_TestBuffer.PaiduiUser), sizeof(User_Type));
					
					//��ϵͳ���������л�ȡ����ʱled������ֵ
					GB_TestBuffer.CurrentTestDataBuffer->ledLight = getTestLedLightIntensity(getGBSystemSetData());
					
					return Error_OK;
				}
			}
		}
		
		return Error_PaiduiFull;
	}
}




ItemData * GetTestItemByIndex(unsigned char index)
{
	return GB_TestBuffer.PaiduiTestDataBuffer[index];
}


unsigned short GetMinWaitTime(void)
{
	unsigned char index = 0;
	unsigned short min = 0xffff;
	unsigned short temp = 0;
	
	for(index = 0; index < PaiDuiWeiNum; index++)
	{
		if((GB_TestBuffer.PaiduiTestDataBuffer[index])&&
			(timerIsStartted(&(GB_TestBuffer.PaiduiTestDataBuffer[index]->timer)))&&
			(false == timerIsStartted(&(GB_TestBuffer.PaiduiTestDataBuffer[index]->timer2))))
		{
			temp = timer_surplus(&(GB_TestBuffer.PaiduiTestDataBuffer[index]->timer));
			if(temp < min)
				min = temp;
		}
	}
	
	return min;
}


/***************************************************************************************************
*FunctionName: SetCurrentTestItem, GetCurrentTestItem
*Description: ��д��ǰ���Ի���
*Input: 
*Output: 
*Return: 
*Author: xsx
*Date: 2016��12��2��15:54:25
***************************************************************************************************/
void SetCurrentTestItem(ItemData * s_itemdata)
{
	GB_TestBuffer.CurrentTestDataBuffer = s_itemdata;
}

ItemData * GetCurrentTestItem(void)
{
	return GB_TestBuffer.CurrentTestDataBuffer;
}

/***************************************************************************************************
*FunctionName: DeleteCurrentTest
*Description: ɾ����ǰ����
*Input: None
*Output: None
*Return: My_Pass -- ɾ���ɹ�
*Author: xsx
*Date: 2016��12��2��15:55:07
***************************************************************************************************/
MyState_TypeDef DeleteCurrentTest(void)
{

	if(GB_TestBuffer.CurrentTestDataBuffer)
	{
		if(GB_TestBuffer.CurrentTestDataBuffer->testlocation == 0)
			GB_TestBuffer.NormalTestDataBuffer = NULL;
		else
			GB_TestBuffer.PaiduiTestDataBuffer[GB_TestBuffer.CurrentTestDataBuffer->testlocation-1] = NULL;

		MyFree(GB_TestBuffer.CurrentTestDataBuffer);
		
		GB_TestBuffer.CurrentTestDataBuffer = NULL;
	}
	return My_Pass;
}

void SetPaiduiUser(User_Type * user)
{
	if(user)
	{
		memcpy(&(GB_TestBuffer.PaiduiUser), user, sizeof(User_Type));
	}
}

void GetPaiduiUser(User_Type * user)
{
	if(user)
	{
		memcpy(user, &(GB_TestBuffer.PaiduiUser), sizeof(User_Type));
	}
}
