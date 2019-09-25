

#ifndef _ISERIAL_H
#define _ISERIAL_H

#include <stddef.h>
#include "ICGui.h"

struct DLL_CLS CISerial
{
	CISerial(){;}
	~CISerial(){;}

	enum
	{
		ERR_NO     = 0,
		ERR_HANDLE = 1,
		ERR_READ   = 2,
		ERR_WRITE  = 3
	};

	virtual int  Read(void*, size_t) = 0;
	virtual int  Write(void*, size_t) = 0;
	virtual int  Error() = 0;
	virtual size_t  SetPos(size_t pos) = 0;
	virtual size_t  GetPos() = 0;
	virtual int  GetValidCount() = 0;
};
typedef int (CISerial::*SERIAL_FUNC)(void*, size_t);


#endif