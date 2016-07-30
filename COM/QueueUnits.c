/***************************************************************************************************
*FileName:QueueUnits
*Description:队列操作工具类
*Author:xsx
*Data:2016年4月22日14:28:03
***************************************************************************************************/

/***************************************************************************************************/
/******************************************头文件***************************************************/
/***************************************************************************************************/
#include	"QueueUnits.h"


/***************************************************************************************************/
/**************************************局部变量声明*************************************************/
/***************************************************************************************************/

/***************************************************************************************************/
/**************************************局部函数声明*************************************************/
/***************************************************************************************************/
static unsigned char WaittingForMutex(xSemaphoreHandle mutex, portTickType xBlockTime);
static void GivexMutex(xSemaphoreHandle mutex);
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************正文********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
static unsigned char WaittingForMutex(xSemaphoreHandle mutex, portTickType xBlockTime)
{
	return xSemaphoreTake(mutex, xBlockTime);
}
static void GivexMutex(xSemaphoreHandle mutex)
{
	xSemaphoreGive(mutex);
}

/***************************************************************************************************
*FunctionName：ReceiveCharFromQueue
*Description：接收一个字节到队列
*Input：queue -- 目标队列
*		mutex -- 此队列的互斥量，可为null
*		sendstr -- 接收数据的存放地址
*		xBlockTime -- 阻塞时间
*Output：None
*Author：xsx
*Data：2016年4月22日15:36:44
***************************************************************************************************/
unsigned char ReceiveCharFromQueue(xQueueHandle queue, xSemaphoreHandle mutex, void * receivedchar , portTickType xBlockTime)
{
	unsigned char * p = (unsigned char *)receivedchar;
	unsigned char statues = pdFAIL;
	
	if(queue == NULL)
		return pdFAIL;
	
	if(mutex != NULL)
	{
		if(pdFAIL == WaittingForMutex(mutex, xBlockTime))
			return pdFAIL;
	}

	statues = xQueueReceive( queue, p, xBlockTime );
	
	if(mutex != NULL)
		GivexMutex(mutex);
	
	return statues;
}

/***************************************************************************************************
*FunctionName：SendCharToQueue
*Description：发送一个字节到队列
*Input：queue -- 目标队列
*		mutex -- 此队列的互斥量，可为null
*		sendstr -- 发送的数据
*		xBlockTime -- 阻塞时间
*Output：None
*Author：xsx
*Data：2016年4月22日15:36:15
***************************************************************************************************/
unsigned char SendCharToQueue(xQueueHandle queue, xSemaphoreHandle mutex, unsigned char receivedchar , portTickType xBlockTime,
	void (*fun)(void))
{
	unsigned char statues = pdFAIL;
	
	if(queue == NULL)
		return pdFAIL;
	
	if(mutex != NULL)
	{
		if(pdFAIL == WaittingForMutex(mutex, xBlockTime))
			return pdFAIL;
	}

	statues = xQueueSend( queue, &receivedchar, xBlockTime );
	
	if(mutex != NULL)
		GivexMutex(mutex);
	
	if(fun != NULL)
		fun();
	
	return statues;
} 

/***************************************************************************************************
*FunctionName：ReceiveStrFromQueue
*Description：从队列接收数据
*Input：queue -- 目标队列
*		mutex -- 此队列的互斥量，可为null
*		sendstr -- 存放接收数据的地址
*		len -- 接收的数据长度（字节长）
*		xBlockTime -- 阻塞时间
*Output：None
*Author：xsx
*Data：2016年4月22日15:35:40
***************************************************************************************************/
unsigned char ReceiveStrFromQueue(xQueueHandle queue, xSemaphoreHandle mutex, void *receivedstr , unsigned short len , 
	portTickType xBlockTime)
{
	unsigned short i=0;
	unsigned char *pdata = (unsigned char *)receivedstr;
	unsigned char statues = pdFAIL;
	
	if(queue == NULL)
		return pdFAIL;
	
	if(mutex != NULL)
	{
		if(pdFAIL == WaittingForMutex(mutex, xBlockTime))
			return pdFAIL;
	}
	
	for(i=0; i<len; i++)
	{
		if(pdPASS == xQueueReceive(queue, pdata , xBlockTime))
		{
			pdata++;
			statues = pdPASS;
		}
		else
		{
			statues = pdFAIL;
			break;
		}
	}
	
	if(mutex != NULL)
		GivexMutex(mutex);
	
	return statues;
}
/***************************************************************************************************
*FunctionName：SendStrToQueue
*Description：发送数据到队列
*Input：queue -- 目标队列
*		mutex -- 此队列的互斥量，可为null
*		sendstr -- 发送的数据地址
*		len -- 发送的数据长度（字节长）
*		xBlockTime -- 阻塞时间
*Output：None
*Author：xsx
*Data：2016年4月22日15:33:38
***************************************************************************************************/
unsigned char SendStrToQueue(xQueueHandle queue, xSemaphoreHandle mutex, void *sendstr , unsigned short len ,  
	portTickType xBlockTime, void (*fun)(void))
{
	unsigned short i=0;
	unsigned char *pdata = (unsigned char *)sendstr;
	unsigned char statues = pdFAIL;
	
	if(queue == NULL)
		return pdFAIL;
	
	if(mutex != NULL)
	{
		if(pdFAIL == WaittingForMutex(mutex, xBlockTime))
			return pdFAIL;
	}

	for(i=0; i<len; i++)
	{
		if(pdPASS == xQueueSend(queue, pdata , xBlockTime))
		{
			pdata++;
			statues = pdPASS;
		}
		else
		{
			statues = pdFAIL;
			break;
		}
	}
	
	if(mutex != NULL)
		GivexMutex(mutex);
	
	if(fun != NULL)
		fun();
	
	return statues;
}


/***************************************************************************************************
*FunctionName：ReceiveDataFromQueue
*Description：从队列中读取有限长度的数据
*Input：queue -- 目标队列
*		mutex -- 此队列的互斥量，可为null
*		receivedstr -- 存放接收数据的地址
*		len -- 接收的数据长度(长度为此队列单元数据大小的个数，与数据接收数据的字节长度无关)
*		xBlockTime -- 阻塞时间
*Output：None
*Author：xsx
*Data：2016年4月22日15:35:40
***************************************************************************************************/
unsigned char ReceiveDataFromQueue(xQueueHandle queue, xSemaphoreHandle mutex, void *receivedstr , unsigned short len ,
	unsigned short itemsize, portTickType xBlockTime)
{
	unsigned short i=0;
	unsigned char *pdata = (unsigned char *)receivedstr;
	unsigned char statues = pdFAIL;
	
	if(queue == NULL)
		return pdFAIL;
	
	if(mutex != NULL)
	{
		if(pdFAIL == WaittingForMutex(mutex, xBlockTime))
			return pdFAIL;
	}
	
	for(i=0; i<len; i++)
	{
		if(pdPASS == xQueueReceive(queue, pdata , xBlockTime))
		{
			pdata += itemsize;
			statues = pdPASS;
		}
		else
		{
			statues = pdFAIL;
			break;
		}
	}
	
	if(mutex != NULL)
		GivexMutex(mutex);
	
	return statues;
}
/***************************************************************************************************
*FunctionName：SendDataToQueue
*Description：发送数据到队列
*Input：queue -- 目标队列
*		mutex -- 此队列的互斥量，可为null
*		sendstr -- 发送的数据地址
*		len -- 接收的数据长度(长度为此队列单元数据大小的个数，与数据接收数据的字节长度�
*		xBlockTime -- 阻塞时间
*Output：None
*Author：xsx
*Data：2016年4月22日15:33:38
***************************************************************************************************/
unsigned char SendDataToQueue(xQueueHandle queue, xSemaphoreHandle mutex, void *sendstr , unsigned short len ,  
	unsigned short itemsize, portTickType xBlockTime, void (*fun)(void))
{
	unsigned short i=0;
	unsigned char *pdata = (unsigned char *)sendstr;
	unsigned char statues = pdFAIL;
	
	if(queue == NULL)
		return pdFAIL;
	
	if(mutex != NULL)
	{
		if(pdFAIL == WaittingForMutex(mutex, xBlockTime))
			return pdFAIL;
	}

	for(i=0; i<len; i++)
	{
		if(pdPASS == xQueueSend(queue, pdata , xBlockTime))
		{
			pdata += itemsize;
			statues = pdPASS;
		}
		else
		{
			statues = pdFAIL;
			break;
		}
	}
	
	if(mutex != NULL)
		GivexMutex(mutex);
	
	if(fun != NULL)
		fun();
	
	return statues;
}

