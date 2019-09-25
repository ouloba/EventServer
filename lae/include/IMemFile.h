
#include "ISerial.h"

#ifndef _MEM_IFILE_H
#define _MEM_IFILE_H

#include "ICGui.h"
#include "FastBufAlloc.h"

class DLL_CLS CIMemFile : public CISerial
{
public:
	enum
	{
		ERR_OVERLOW = 4
	};

	virtual int   Read(void* pBuf, size_t nSize);
	virtual int   Write(void* pBuf, size_t nSize);
	virtual int   Error();	
	virtual size_t   SetPos(size_t pos);
	virtual size_t   GetPos()         { return m_nCurPos; }
	virtual int   GetValidCount()  { return m_FastBuf.cplen(); }
	virtual unsigned char* GetBuffer()      { return m_FastBuf.buf(); }
	virtual int   GetBufferLen()   { return m_FastBuf.alloclen(); }

	CIMemFile(void* pBuf, int nLen);
	CIMemFile();
	~CIMemFile();

	void  Initial(void* pBuf, int nLen);
	void Release(int nReverSize = 128);
	void removeHeadBuffer(size_t size);
	const unsigned char* grown(size_t size){ return m_FastBuf.grow(size); }

private:
//	char* m_pOrgBuf;
	FastBufAlloc<unsigned char, 32> m_FastBuf;
//	int   m_nBufLen;
	size_t   m_nCurPos;
	int   m_nErrorCode;
//	int   m_nValidCount;
//	int   m_IsAllocSelft;
};

#endif