/***************************************************************************************************
*FileName:	MyStack
*Description:	栈类型
*Author: xsx_kair
*Data:	2016年12月19日16:49:16
***************************************************************************************************/

/***************************************************************************************************/
/******************************************Header List********************************************/
/***************************************************************************************************/
#include	"MyStack.h"

#include	"MyMem.h"


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

/***************************************************************************************************
*FunctionName: InitLinkStack
*Description: 初始化栈
*Input: 
*Output: 
*Return: 
*Author: xsx
*Date: 2016年12月19日17:09:22
***************************************************************************************************/
void InitLinkStack(LinkStack * linkStack)
{
	if(linkStack)
	{
		linkStack->top = NULL;
	}
}	

/***************************************************************************************************
*FunctionName: StackPush
*Description: 入栈操作
*Input: 
*Output: 
*Return: 
*Author: xsx
*Date: 2016年12月20日08:44:41
***************************************************************************************************/
MyState_TypeDef StackPush(LinkStack * linkStack, Activity * activity)
{
	StackNode * stackNode = NULL;
	
	//不允许入栈空值
	if(NULL == activity)
		return My_Fail;
	
	stackNode = MyMalloc(sizeof(StackNode));
	
	if(stackNode)
	{
		//入栈，先隐藏当前页面
		if((linkStack->top) && (linkStack->top->activity) && (linkStack->top->activity->pageHide))
			linkStack->top->activity->pageHide();
		
		stackNode->activity = activity;
		stackNode->lastStackNode = linkStack->top;
		
		linkStack->top = stackNode;
		
		return My_Pass;
	}
	else
		return My_Fail;
}

/***************************************************************************************************
*FunctionName: StackPop
*Description: 出栈操作
*Input: 
*Output: 
*Return: 
*Author: xsx
*Date: 2016年12月20日08:50:28
***************************************************************************************************/
MyState_TypeDef StackPop(LinkStack * linkStack, MyBool isResume)
{
	StackNode * stackNode = NULL;
	
	//栈空，出栈失败
	if(linkStack->top == NULL)
		return My_Fail;

	//不空
	else
	{
		//销毁activity页面数据
		if((linkStack->top->activity) && (linkStack->top->activity->pageDestroy))
			linkStack->top->activity->pageDestroy();
		
		//销毁activity
		MyFree(linkStack->top->activity);
		
		stackNode = linkStack->top;
		
		//恢复新节点
		linkStack->top = linkStack->top->lastStackNode;
		if((linkStack->top) && (linkStack->top->activity) && (linkStack->top->activity->pageResume) && (isResume))
			linkStack->top->activity->pageResume();

		//销毁出栈节点
		MyFree(stackNode);
		
		return My_Pass;
	}
}

/***************************************************************************************************
*FunctionName: StackTop
*Description: 获取栈顶节点的数据
*Input: 
*Output: 
*Return: 
*Author: xsx
*Date: 2016年12月20日10:11:25
***************************************************************************************************/
MyState_TypeDef StackTop(LinkStack * linkStack, Activity ** activity)
{

	//栈空，出栈失败
	if(linkStack->top == NULL)
		return My_Fail;

	//不空
	else
	{
		if(activity)
			*activity = linkStack->top->activity;
		
		return My_Pass;
	}
}
/****************************************end of file************************************************/
