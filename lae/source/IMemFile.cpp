
#include "stdio.h"
#include "IMemFile.h"
#include "memory.h"
#include <string.h>

CIMemFile::CIMemFile(void* pBuf, int nLen)
{
	m_FastBuf.grow(nLen);
	memcpy(m_FastBuf.buf(), pBuf, nLen);
	m_FastBuf.setcplen(nLen);
	m_nCurPos    = 0;
	m_nErrorCode = ERR_NO;
}

CIMemFile::CIMemFile()
{
	m_nCurPos    = 0;
	m_nErrorCode = ERR_NO;
}

CIMemFile::~CIMemFile()
{	
	m_FastBuf.release();
}

void CIMemFile::Release(int nReverSize)
{
	m_FastBuf.release();
	m_nCurPos    = 0;
	m_nErrorCode = ERR_NO;
}

void CIMemFile::Initial(void* pBuf, int nLen)
{
	m_FastBuf.grow(nLen);
	memcpy(m_FastBuf.buf(), pBuf, nLen);
	m_FastBuf.setcplen(nLen);
	m_nErrorCode = ERR_NO;
}

int CIMemFile::Read(void* pBuf, size_t nSize)
{
	if(m_nCurPos + nSize <= m_FastBuf.cplen())
	{
		memcpy(pBuf, m_FastBuf.buf()+m_nCurPos, nSize);
		m_nCurPos    += nSize;
		return nSize;
	}
	else
	{
		m_nErrorCode = ERR_READ;
	}

	return -1;
}

size_t CIMemFile::SetPos(size_t pos)
{
	if (pos <= m_FastBuf.cplen())
		 m_nCurPos = pos;
	 return pos;
}

void CIMemFile::removeHeadBuffer(size_t size){
	if(size>=m_FastBuf.cplen()){
		m_FastBuf.setcplen(0);
		m_nCurPos=0;
		return;
	}

	memmove(m_FastBuf.buf(), m_FastBuf.buf()+size,m_FastBuf.cplen()-size);
	m_FastBuf.setcplen(m_FastBuf.cplen()-size);
	m_nCurPos=0;
}

int CIMemFile::Write(void* pBuf, size_t nSize)
{
	if(m_nCurPos+nSize <= m_FastBuf.alloclen())
	{
		memcpy(m_FastBuf.buf()+m_nCurPos, pBuf, nSize);		
		m_nCurPos += nSize;
		
		if(m_nCurPos>m_FastBuf.cplen()) 
		{
			//m_nValidCount = m_nCurPos;
			m_FastBuf.setcplen(m_nCurPos);
		}

		return nSize;
	}
	else
	{
		m_FastBuf.grow((m_FastBuf.alloclen()+nSize)*2);
		memcpy(m_FastBuf.buf()+m_nCurPos, pBuf, nSize);	
		m_nCurPos    += nSize;
		if(m_nCurPos>m_FastBuf.cplen()) 
		{
			m_FastBuf.setcplen(m_nCurPos);
		}
	}

	return -1;
}

int CIMemFile::Error()
{
	return m_nErrorCode;;
}
