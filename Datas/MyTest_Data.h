#ifndef _MYTEST_S_D_H__
#define _MYTEST_S_D_H__

#include	"Define.h"
#include	"Motor_Data.h"
#include	"Timer_Data.h"
#include	"Temperature_Data.h"

#pragma pack(1)
typedef struct TestLine_tag {
	unsigned short TestPoint[MaxPointLen];
	unsigned short C_Point[2];
	unsigned short T_Point[2];
	unsigned short B_Point[2];
	float BasicBili;
	float BasicResult;
	float AdjustResult;
} TestLine;
#pragma pack()

#pragma pack(1)
typedef struct TestData_tag {
	User_Type user;
	char sampleid[MaxSampleIDLen];
	CardCodeInfo temperweima;
	TestLine testline;
	MyTime_Def TestTime;
	MyTemp_Def TestTemp;
	unsigned short time;					//��ʱʱ��
	unsigned short crc;
}TestData;
#pragma pack()

typedef struct ItemData_tag {
	TestData testdata;
	Timer timer;																		//���浹��ʱ��ʱ��
	Timer timer2;																		//��ʱ����ʱ��ʱ��
	Timer timer3;																		//��ʱ����ʱ��ʱ��
	unsigned char errorcount;															//δ��������
	MyPaiDuiStatues statues;															//���Խ׶�
	unsigned char jieduan;																//���Խ׶�
	unsigned char testlocation;
}ItemData;

unsigned char GetUsableLocation(void);
unsigned char CreateANewTest(unsigned char index);
MyState_TypeDef DeleteCurrentTest(void);
ItemData * GetTestItemByIndex(unsigned char index);
unsigned short GetMinWaitTime(void);

void SetCurrentTestItem(ItemData * s_itemdata);
ItemData * GetCurrentTestItem(void);
#endif