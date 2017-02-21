/***************************************************************************************************
*FileName: UserDao
*Description: ������dao
*Author: xsx_kair
*Data: 2017��2��16��11:31:22
***************************************************************************************************/

/***************************************************************************************************/
/******************************************Header List********************************************/
/***************************************************************************************************/
#include	"UserDao.h"

#include	"CRC16.h"
#include	"MyMem.h"

#include	"ff.h"

#include	<string.h>
#include	"stdio.h"
#include 	"stdlib.h"
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/


/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/****************************************File Start*************************************************/
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
	unsigned char i=0;
	
	myfile = MyMalloc(sizeof(FatfsFileInfo_Def));

	if(myfile && user)
	{
		memset(myfile, 0, sizeof(FatfsFileInfo_Def));

		myfile->res = f_open(&(myfile->file), "0:/Testers.ncd", FA_READ);
		
		if(FR_OK == myfile->res)
		{
			f_lseek(&(myfile->file), 0);
					
			for(i=0; i<MaxUserNum; i++)
			{
				f_read(&(myfile->file), user, sizeof(User_Type), &(myfile->br));
				
				if(user->crc == CalModbusCRC16Fun1(user, sizeof(User_Type)-2))
					user++;
			}
			
			statues = My_Pass;
			
			f_close(&(myfile->file));
		}
	}	
	MyFree(myfile);
	
	return statues;
}

MyState_TypeDef ClearUsers(void)
{
	User_Type * user = NULL;
	user = MyMalloc(sizeof(User_Type)*MaxUserNum);
	if(user)
	{
		memset(user, 0, sizeof(User_Type)*MaxUserNum);
		
		return SaveUserData(user);
	}
	else
		return My_Fail;
}

/****************************************end of file************************************************/