#ifndef __NETSET_P_H__
#define __NETSET_P_H__

#include	"Define.h"

typedef struct NetSetPageBuffer_tag {
	NetData myNetData;
	char buf[100];
	unsigned char ischanged;
}NetSetPageBuffer;


unsigned char DspNetSetPage(void *  parm);

#endif
