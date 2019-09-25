

#include "IArchive.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#ifndef WIN32
#include <time.h>   
#include<sys/time.h>
#include <unistd.h>
#endif
/*
static short  (*Short_Swap)(short);
static float  (*Float_Swap)(const float*);
static int    (*Long_Swap)(int);
static LXZuint64   (*LongLong_Swap)(LXZuint64);
*/
short ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short	ShortNoSwap (short l)
{
	return l;
}

LXZuint64 LongLongSwap(LXZuint64 l)
{
	byte    b1,b2,b3,b4,b5,b6,b7,b8;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;
	b5 = (l>>32)&255;
	b6 = (l>>40)&255;
	b7 = (l>>48)&255;
	b8 = (l>>56)&255;

	return ((LXZuint64)b1<<56)+((LXZuint64)b2<<48)+((LXZuint64)b3<<40)+((LXZuint64)b4<<32)+((LXZuint64)b5<<24)+((LXZuint64)b6<<16)+((LXZuint64)b7<<8)+b8;
}

LXZuint64 LongLongNotSwap(LXZuint64 l)
{
	return l;
}

int    LongSwap (int l)
{
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

int	LongNoSwap (int l)
{
	return l;
}

typedef union {
    float	f;
    unsigned int i;
} _FloatByteUnion;

float FloatSwap (const float *f) {
	const _FloatByteUnion *in;
	_FloatByteUnion out;

	in = (_FloatByteUnion *)f;
	out.i = LongSwap(in->i);

	return out.f;
}

float FloatNoSwap (const float *f)
{
	return *f;
}

void CIArchive::SetLittleEndian(bool isLittelEndian) {
	union
	{
		long l;
		unsigned char c[4];
	} u;
	u.l = 0x12345678;
	
	if ((isLittelEndian&&u.c[0] == 0x12)|| (!isLittelEndian&&u.c[0] == 0x78)) {
		Short_Swap = ShortSwap;
		Float_Swap = FloatSwap;
		Long_Swap = LongSwap;
		LongLong_Swap = LongLongSwap;
	}	
	else {
		Short_Swap = ShortNoSwap;
		Float_Swap = FloatNoSwap;
		Long_Swap = LongNoSwap;
		LongLong_Swap = LongLongNotSwap;
	}
}

//static int IsLittelEndian = 2;
CIArchive::CIArchive()
{
	m_byVersion = LXZWINDOW_VER;
	m_pISerial   = 0;
	m_SerialFunc = 0;
	m_nMode      = SM_READ;
	m_nExeCnt = 0;
	m_pArchive = NULL;

	memset(m_arrAddress, 0, sizeof(m_arrAddress));	
	m_nIndex = 0;

	Short_Swap = ShortNoSwap;
	Float_Swap = FloatNoSwap;
	Long_Swap = LongNoSwap;	
	LongLong_Swap = LongLongNotSwap;	
}

CIArchive::CIArchive(CISerial* pISerial, ISerialMode nMode)
{
	m_byVersion = LXZWINDOW_VER;
	m_pISerial   = pISerial;
	m_SerialFunc = (nMode == SM_READ)?&CISerial::Read:&CISerial::Write;
	m_nMode      = nMode;
	m_nExeCnt = 0;
	m_pArchive = NULL;

	memset(m_arrAddress, 0, sizeof(m_arrAddress));
	m_nIndex = 0;

	Short_Swap = ShortNoSwap;
	Float_Swap = FloatNoSwap;
	Long_Swap = LongNoSwap;
	LongLong_Swap = LongLongNotSwap;
}

CIArchive::~CIArchive()
{
	if(m_pArchive) 
		delete m_pArchive;
}

uiboolean CIArchive::IsReading()
{
	if(m_nMode == SM_READ)
	{
		return uitrue;
	}
	else
	{
		return uifalse;
	}
}

uiboolean CIArchive::IsWriting()
{
	if(m_nMode == SM_WRITE)
	{
		return uitrue;
	}
	else
	{
		return uifalse;
	}
}



#define  SAVE_ADDRESS() 
void CIArchive::Serial(int& l)
{
	if(IsReading())
	{
		int v=0;
		int ret = (m_pISerial->*m_SerialFunc)(&v, sizeof(int));
		if (ret>0) l = Long_Swap(v);
	}
	else if(IsWriting())
	{

		SAVE_ADDRESS();
		
		int v = l;
		Long_Swap(v);
		(m_pISerial->*m_SerialFunc)(&v, sizeof(int));
	}	
}

void CIArchive::Serial(uiboolean& l)
{
	if(IsReading())
	{
		int v=0;
		int ret = (m_pISerial->*m_SerialFunc)(&v, sizeof(int));
		if (ret>0) l = (Long_Swap(v) == 1) ? uitrue : uifalse;
	}
	else if(IsWriting())
	{		
		SAVE_ADDRESS();
		int v = l;
		Long_Swap(v);
		(m_pISerial->*m_SerialFunc)(&v, sizeof(int));
	}	
}

void CIArchive::Serial(bool& l)
{
	if(IsReading())
	{
		int v=0;
		int ret = (m_pISerial->*m_SerialFunc)(&v, sizeof(int));
		if (ret>0) l = (Long_Swap(v) == 1) ? true : false;
	}
	else if(IsWriting())
	{
		SAVE_ADDRESS();

		int v = l?1:0;
		Long_Swap(v);
		(m_pISerial->*m_SerialFunc)(&v, sizeof(int));
	}	
}

void CIArchive::Serial(float& l)
{
	typedef union
	{
		float f;
		long  l;
	}_fl;

	if(m_nMode == SM_READ)
	{
		_fl fl;
		int ret = (m_pISerial->*m_SerialFunc)(&fl.l, sizeof(float));
		fl.l = Long_Swap(fl.l);
		if (ret>0) l = fl.f;
	}
	else
	{
		SAVE_ADDRESS();

		_fl fl;
		fl.f = l;
		Long_Swap(fl.l);
		(m_pISerial->*m_SerialFunc)(&fl.l, sizeof(float));
	}
}

void CIArchive::Serial(long& l)
{
	if(m_nMode == SM_READ)
	{
		long v=0;
		int ret = (m_pISerial->*m_SerialFunc)(&v, sizeof(long));
		if (ret>0) l = Long_Swap(v);
	}
	else
	{
		SAVE_ADDRESS();

		long v = l;
		Long_Swap(v);
		(m_pISerial->*m_SerialFunc)(&v, sizeof(long));
	}
}

void CIArchive::Serial(LXZsint8& l)
{
	SAVE_ADDRESS();
	(m_pISerial->*m_SerialFunc)(&l, sizeof(LXZsint8));
}

void CIArchive::Serial(LXZuint8& l)
{
	SAVE_ADDRESS();
	(m_pISerial->*m_SerialFunc)(&l, sizeof(LXZuint8));
}
/*

void CIArchive::Serial(unsigned char& l)
{
	Test(sizeof(int));
	(m_pISerial->*m_SerialFunc)(&l, sizeof(LXZuint8));
}
*/


void CIArchive::Serial(LXZsint16& l)
{
	if(m_nMode == SM_READ)
	{
		LXZsint16 v=0;
		int ret = (m_pISerial->*m_SerialFunc)(&v, sizeof(LXZsint16));
		if (ret>0) l = Short_Swap(v);
	}
	else
	{
		SAVE_ADDRESS();
		LXZsint16 v = l;
		Short_Swap(v);
		(m_pISerial->*m_SerialFunc)(&v, sizeof(LXZsint16));
	}
}


void CIArchive::Serial(LXZuint16& l)
{
	if(m_nMode == SM_READ)
	{
		LXZsint16 v=0;
		int ret = (m_pISerial->*m_SerialFunc)(&v, sizeof(LXZsint16));
		if (ret>0) l = Short_Swap(v);
	}
	else
	{
		SAVE_ADDRESS();
		LXZsint16 v = l;
		Short_Swap(v);
		(m_pISerial->*m_SerialFunc)(&v, sizeof(LXZsint16));
	}
}
/*

void CIArchive::Serial(sint32& l)
{
	if(m_nMode == SM_READ)
	{
		sint32 v=0;
		(m_pISerial->*m_SerialFunc)(&v, sizeof(sint32));
		l = Long_Swap(v);
	}
	else
	{
		Test(sizeof(int));
		sint32 v = l;
		Long_Swap(v);
		(m_pISerial->*m_SerialFunc)(&v, sizeof(sint32));
	}
}*/


void CIArchive::Serial(LXZuint32& l)
{
	if(m_nMode == SM_READ)
	{
		LXZuint32 v=0;
		int ret = (m_pISerial->*m_SerialFunc)(&v, sizeof(LXZuint32));
		if (ret>0) l = Long_Swap(v);
	}
	else
	{
		SAVE_ADDRESS();
		LXZuint32 v = l;
		Long_Swap(v);
		(m_pISerial->*m_SerialFunc)(&v, sizeof(LXZuint32));
	}
}

void CIArchive::Serial(LXZuint64& l)
{
	if(m_nMode == SM_READ)
	{
		LXZuint64 v=0;
		int ret = (m_pISerial->*m_SerialFunc)(&v, sizeof(LXZuint64));
		if (ret>0) l = LongLong_Swap(v);
	}
	else
	{
		SAVE_ADDRESS();
		LXZuint64 v = l;
		LongLong_Swap(v);
		(m_pISerial->*m_SerialFunc)(&v, sizeof(LXZuint64));
	}
}

int CIArchive::getError()
{
	return m_pISerial->Error();
}


void CIArchive::Serial(char* str,
					   int len,
					   int nMaxBuf)
{
	// assert(len <= nMaxBuf);
	if(IsReading())
	{
		char* p = str;
		for(int i = 0; i < nMaxBuf; i++)
			*p++=0;

		int l = 0;
		Serial(l);
		if(l <= nMaxBuf)
		{
			(m_pISerial->*m_SerialFunc)(str, l);
		}
	}
	else
	{
		SAVE_ADDRESS();
		Serial(len);
		(m_pISerial->*m_SerialFunc)(str, len);				
	}
}

void CIArchive::Serial(char*& str)
{
	if(IsReading())
	{
		int len = 0;
		Serial(len);
		char* s = new char[len+1];
		Serial(s, len);
		s[len]=0;
		str = s;
	}
	else
	{
		SAVE_ADDRESS();
		int len = strlen(str);
		Serial(len);
		Serial(str, len);
	}
}

void CIArchive::Serial(char* str, int len)
{
	// assert(len <= nMaxBuf);
	if(IsReading())
	{
		(m_pISerial->*m_SerialFunc)(str, len);
	}
	else
	{
		SAVE_ADDRESS();
		(m_pISerial->*m_SerialFunc)(str, len);				
	}
}

void CIArchive::Write(const char* buf, int len){
	if (IsReading() == uitrue)
		return;

	char* p = const_cast<char*>(buf);
	(m_pISerial->*m_SerialFunc)(p, len);

}

char* CIArchive::SerialStr(char* str, int len)
{
	if(IsReading())
	{
		int l=0;
		Serial(l);
		if(l<len)
		(m_pISerial->*m_SerialFunc)(str, l);
		str[l]=0;
		return str;
	}
	else
	{
		SAVE_ADDRESS();
		int len = strlen(str);
		Serial(len);
		(m_pISerial->*m_SerialFunc)(str, len);
		return str;
	}
}

void CIArchive::Initial(CISerial* pISerial, int nMode)
{
	pISerial->SetPos(0);
	m_pISerial   = pISerial;
	m_SerialFunc = (nMode == SM_READ)?&CISerial::Read:&CISerial::Write;
	m_nMode      = (nMode==SM_READ)?SM_READ:SM_WRITE;	
}
	
void CIArchive::setMode(int nMode)
{
	m_pISerial->SetPos(0);
	m_SerialFunc = (nMode == SM_READ)?&CISerial::Read:&CISerial::Write;
	m_nMode      = (nMode==SM_READ)?SM_READ:SM_WRITE;	
}
	
void CIArchive::PushAddress()
{
	if((m_nIndex<10)&&IsWriting())
	{
		m_arrAddress[m_nIndex] = m_pISerial->GetPos();
		m_nIndex++;
	}
}

void CIArchive::SerialPtr(void*& ptr)
{
	if(IsReading()){
		LXZuint64 v;
		Serial(v);
		ptr = (void*)v;
	}
	else
	{
		LXZuint64 v = (LXZuint64)ptr;
		Serial(v);
	}
}

void CIArchive::SerialPtr(void*& ptr, const char* name)
{	
	//assert(strlen(name)<32)
	int len = strlen(name);

	if(IsReading())
	{	
		char buf[32] = {'\0'};
		Serial(buf, 32, 32);
		if(len<32)
		{
		//	assert(strcmp(buf, name)==0);
			LXZuint64 v;
			Serial(v);
			if(strcmp(buf,name)==0)	ptr = (void*)v;
			else ptr = NULL;
		}
		else
		{
			LXZuint64 v;
			Serial(v);
			ptr = (void*)v;
		}
	}
	else
	{
		
		if(len<32)
		{
			char buf[32] = {'\0'};
			strcpy(buf, name);
			Serial(buf, 32, 32);
		}

		LXZuint64 v = (LXZuint64)ptr;
		Serial(v);
	}
}

size_t CIArchive::setPos(size_t pos)
{
	return m_pISerial->SetPos(pos);
}

size_t CIArchive::getPos()
{
	return m_pISerial->GetPos();
}

void CIArchive::setIndexPos(int index)
{
	if(index>=0&&index<10){
		setPos(m_arrAddress[index]);
	}
	else{
#ifndef WIN32
		struct timeval tv;
		char mytime[20] = {0};
		gettimeofday(&tv,NULL);
		strftime(mytime,sizeof(mytime),"%Y-%m-%d %T",localtime(&tv.tv_sec));
		FILE* file = fopen("setIndexPos.txt","a+");
		if(file){
			fprintf(file,"[%s] exit signal:%d",mytime, 0);
			fclose(file);
		}
#endif

	}
}
