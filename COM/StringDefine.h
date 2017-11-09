#ifndef _STRINGDEFINE_H_H
#define _STRINGDEFINE_H_H

#include	"Define.h"

#define		AdminPassWord		"201300\0"								//����Ա���룬�����޸��豸id
#define		TestPassWord		"201302\0"								//�ϻ���������
#define		CheckQRPassWord		"201303\0"								//���Զ�ά������
#define		AdjLedPassWord		"201304\0"								//У׼led���롤����������������������������������
#define		FactoryResetPassWord	"201305\0"							//�ָ�������������
#define		ChangeValueShowTypePassWord	"201306\0"						//�л������ʾģʽ���Ƿ���ʾ��ʵֵ
#define		UnlockLCDPassWord	"201307\0"								//������Ļһ��

#if(DeviceLanguage == DEVICE_CN)
	#define	DeviceNameStr 		"ӫ�����߶���������\0"
	#define	CompanyNameStr		"�人Ŧ��������Ƽ��ɷ����޹�˾\0"
	#define	TesterNameStr		"������\0"
	#define	SampleIdStr			"��Ʒ���\0"
	#define	ItemNameStr			"������Ŀ\0"
	#define	ResultStr			"���Խ��\0"
	#define	ReferenceValueStr	"�ο�ֵ\0"
	#define	TestTimeStr			"����ʱ��\0"
	#define	PrintTimeStr		"��ӡʱ��\0"
	#define	StatementStr		"����: ��������Ա��걾����!\0"
#elif(DeviceLanguage == DEVICE_EN)
	#define	DeviceNameStr 		"Fluor Immun Quant Analyzer\0"
	#define	CompanyNameStr		"Wuhan Newcando Biotechnology Co.,Ltd.\0"
	#define	TesterNameStr		"Tester\0"
	#define	SampleIdStr			"SampleId\0"
	#define	ItemNameStr			"Item\0"
	#define	ResultStr			"Result\0"
	#define	ReferenceValueStr	"Reference\0"
	#define	TestTimeStr			"Test Time\0"
	#define	PrintTimeStr		"Print Time\0"
	#define	StatementStr		"Statement: This result is only responsible for this specimen!\0"
#endif

#endif
