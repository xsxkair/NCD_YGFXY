/***************************************************************************************************
*FileName:
*Description:
*Author: xsx_kair
*Data:
***************************************************************************************************/

/***************************************************************************************************/
/******************************************Header List********************************************/
/***************************************************************************************************/
#include	"LwipServerForLab_Task.h"
#include	"LabServer_Fun.h"
#include 	"tcpip.h"

#include 	"FreeRTOS.h"
#include 	"task.h"
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
#define LwipLabServerTask_PRIORITY			2							//客户端任务优先级
const char * LwipLabServerTaskName = "vLwipLabServerTask";				//客户端任务名

struct netconn *pxServerListener, *pxNewConnection;
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
static void LwipLabServerTask(void *pvParameters);
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/****************************************File Start*************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

void StartLwipLabServerTask(void)
{
	xTaskCreate( LwipLabServerTask, LwipLabServerTaskName, configMINIMAL_STACK_SIZE, NULL, LwipLabServerTask_PRIORITY, NULL );
}


static void LwipLabServerTask(void *pvParameters)
{
	err_t err;
	/* Create a new tcp connection handle */
	pxServerListener = netconn_new( NETCONN_TCP );
	
	netconn_bind(pxServerListener, IP_ADDR_ANY, 9001 );
	netconn_listen( pxServerListener );
	
	while(1)
	{
		err = netconn_accept(pxServerListener, &pxNewConnection);

		if(err == ERR_OK)
		{
			pxNewConnection->recv_timeout = (3000 / portTICK_RATE_MS);
			
			ProcessQuest(pxNewConnection);
			
			netconn_close( pxNewConnection );
			netconn_delete( pxNewConnection );
		}
	}
}

/****************************************end of file************************************************/
