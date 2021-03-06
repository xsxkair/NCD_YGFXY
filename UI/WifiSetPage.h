#ifndef __WIFISET_P_H__
#define __WIFISET_P_H__

#include	"Define.h"

#if (DEVICE_CON_TYPE == DEVICE_WIFI)

#include	"UI_Data.h"
#include	"SystemSet_Data.h"

typedef struct WifiPageBuffer_tag {
	unsigned char pageindex;							//页面索引
	unsigned char selectindex;
	WIFI_Def wifilist[MaxWifiListNum];
	WIFI_Def * wifip;
	unsigned short wifinum;
	Basic_ICO myico[PageWifiNum];
	unsigned short lcdinput[50];
	MyBool isGetWifiControl;								//是否已获取wifi控制权
	char buf[100];
}WifiPageBuffer;

MyState_TypeDef createWifiSetActivity(Activity * thizActivity, Intent * pram);

#endif //DEVICE_CON_TYPE

#endif

